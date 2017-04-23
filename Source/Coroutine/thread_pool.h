#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

#include <experimental/coroutine>

namespace sqlpp
{

	template<typename T>
	struct coroutine_future;

	class thread_pool {
	public:
		thread_pool();
		thread_pool(size_t);
		~thread_pool();

		void spawn_thread(size_t count);

		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
			->sqlpp::coroutine_future<typename std::result_of<F(Args...)>::type>;
	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;
		std::atomic<size_t> free_worker_count;

		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};

	inline thread_pool::thread_pool() : stop(false), free_worker_count(0) {}
	inline thread_pool::thread_pool(size_t thread_count) : stop(false), free_worker_count(0)
	{
		spawn_thread(thread_count);
	}

	// the destructor joins all threads
	inline thread_pool::~thread_pool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
		{
			worker.join();
		}
	}

	void thread_pool::spawn_thread(size_t thread_count = 1)
	{
		for (size_t i = 0; i < thread_count; ++i)
		{
			workers.emplace_back([this]
			{
				for (;;)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock, [this]
						{
							return this->stop || !this->tasks.empty();
						});

						if (this->stop && this->tasks.empty())
						{
							return;
						}

						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					free_worker_count--;
					task();
					free_worker_count++;
				}
			});
			free_worker_count++;
		}
	}


	template<class F, class... Args>
	struct query_future
	{
		using return_type = typename std::result_of<F(Args...)>::type;
		std::q_future<return_type> result;
	};

	// add new work item to the pool
	template<class F, class... Args>
	auto thread_pool::enqueue(F&& f, Args&&... args)
		-> sqlpp::coroutine_future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		auto result = sqlpp::coroutine_future<return_type>(task->get_future());

		if (free_worker_count.load() == 0)
		{
			spawn_thread();
		}

		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			if (stop)
			{
				throw std::runtime_error("cannot enqueue on a stopped thread_pool");
			}



			tasks.emplace([task]() { (*task)(); });
		}

		condition.notify_one();
		return result;
	}
}

#endif
