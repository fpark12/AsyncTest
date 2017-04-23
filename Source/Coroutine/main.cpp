

#include <iostream>
#include <string>
#include <sstream>
#include <future>
#include <thread>
#include <memory>

#include <experimental/coroutine>

#include "thread_pool.h"


struct resumable
{
	struct promise_type
	{
		int _value;

		resumable get_return_object()
		{
			return resumable(std::experimental::coroutine_handle<promise_type>::from_promise(*this));
		}

		auto initial_suspend()
		{
			return std::experimental::suspend_never{};
		}

		auto final_suspend()
		{
			return std::experimental::suspend_always{};
		}

		//*
		auto return_void()
		{
		}
		//*/

		/*
		auto return_value(int value)
		{
			_value = value;
		}
		//*/
	};

	int get()
	{
		return _coroutine.promise()._value;
	}

	std::experimental::coroutine_handle<promise_type> _coroutine = nullptr;

	explicit resumable(std::experimental::coroutine_handle<promise_type> coroutine)
		: _coroutine(coroutine)
	{
	}

	~resumable()
	{
		if (_coroutine)
		{
			_coroutine.destroy();
		}
	}

	void resume()
	{
		_coroutine.resume();
	}

	resumable() = default;
	resumable(resumable const&) = delete;
	resumable& operator= (resumable const&) = delete;

	resumable(resumable && other)
		: _coroutine(std::move(other._coroutine))
	{
	}

	resumable& operator=(resumable&& other)
	{
		_coroutine = std::move(other._coroutine);
	}
};

resumable named_counter(std::string name)
{
	std::cerr << "counter(" << name << ") was called" << std::endl;
	for (int i = 0; i < 5; i++)
	{
		co_await std::experimental::suspend_always{};
		std::cerr << "counter(" << name << ") resumed #" << i << std::endl;
	}
}

std::string thread_id()
{
	auto id = std::this_thread::get_id();
	std::stringstream id_text;
	id._To_text(id_text);
	return id_text.str();
}

struct worker
{
	bool done = false;

	std::q_future<void> work()
	{
		std::cerr << "Start working, thread # " << thread_id() << std::endl;
		co_await std::async(std::launch::async, [&]
		{
			std::cerr << "Working, thread # " << thread_id() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			done = true;
		});
		//std::cerr << std::endl;

		std::cerr << "Done working, thread # " << thread_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}

	std::q_future<int> compute()
	{
		std::cerr << "Start computing, thread # " << thread_id() << std::endl;
		int result = co_await std::async(std::launch::async, [&]
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			return 30;
		});
		std::cerr << std::endl;

		std::cerr << "Done computing, thread # " << thread_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(20000));
		co_return result;
	}
};

void test1()
{
	worker w;
	auto f = w.work();

	while (!w.done)
	{
		//std::cerr << ".";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cerr << std::endl;
}

void test2()
{
	worker w;
	auto f2 = w.compute();
	while (!f2._Is_ready())
	{
		std::cerr << ".";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cerr << std::endl;
	std::cerr << "Done computing" << std::endl;


	auto r = f2.get();
	std::cerr << "Result:" << r << std::endl;
}

void test3()
{
	auto a = named_counter("a");
	auto b = named_counter("b");
	a.resume();
	b.resume();
	a.resume();
	b.resume();
}

namespace sqlpp
{
	namespace impl
	{
		sqlpp::thread_pool thread_pool;
	}

	std::mutex coro_mutex;

	template <typename T>
	struct enqueue_on_suspend
	{
		coroutine_future<T> q_future;

		enqueue_on_suspend(coroutine_future<T>&& f)
			: q_future(coroutine_future<T>(std::q_future<T>()))
		{
			q_future = std::move(f);
		}
		bool await_ready() _NOEXCEPT
		{
			return false;
		}

		void await_suspend(std::experimental::coroutine_handle<> coroutine) _NOEXCEPT
		{
			sqlpp::impl::thread_pool.enqueue([&, coroutine]
			{
				{
					std::lock_guard<std::mutex> lock(coro_mutex);
					std::cerr << "Observer, thread # " << thread_id() << std::endl;
				}
				q_future.get();
				coroutine();
			});
			return;
		}

		void await_resume() _NOEXCEPT
		{
			return;
		}
	};

	template<typename T>
	struct coroutine_future : public std::q_future<T>
	{
		coroutine_future(std::q_future<T>&& f)
			: std::q_future<T>(std::move(f))
		{
		}
	};


	template <typename T>
	inline auto operator co_await(coroutine_future<T> f)
	{
		//return std::experimental::suspend_never{};
		return enqueue_on_suspend<T>(std::move(f));
	}

	//inline auto operator co_await

	auto async_query(std::string s)
	{
		return sqlpp::impl::thread_pool.enqueue([s]
		{
			{

				std::lock_guard<std::mutex> lock(coro_mutex);
				std::cerr << s << " on async work, thread # " << thread_id() << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			return 1;
		});
	}

	using namespace std::chrono_literals;
	resumable update_user_info(std::string s)
	{
		//co_await query_future{};
		//co_await 500ms;// then(sqlpp::async_query(), [](int i) {print_thread_id; });
		co_await async_query(s);

		{
			std::lock_guard<std::mutex> lock(coro_mutex);
			std::cerr << s << " on continuation, thread # " << thread_id() << std::endl;
		}
	};
}

/*
static void __stdcall callback(PTP_CALLBACK_INSTANCE, void * context, PTP_TIMER) noexcept
{
	std::experimental::coroutine_handle<>::from_address(context)();
}
*/

int main(int argc, char *argv[])
{
	auto coro = sqlpp::update_user_info("a");
	auto coro1 = sqlpp::update_user_info("b");
	auto coro2 = sqlpp::update_user_info("c");
	auto coro3 = sqlpp::update_user_info("d");
	/*
	auto coro4 = sqlpp::update_user_info();
	auto coro12 = sqlpp::update_user_info();
	auto coro123 = sqlpp::update_user_info();
	*/
	//sqlpp::update_user_info();
	//coro.resume();

	//test1();
	getchar();

	return 0;
}
