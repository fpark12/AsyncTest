#include <memory>
#include <iostream>

#include <mysql.h>
#include <sqlpp11/sqlpp11.h>

#include "connection_handle.h"
#include "sqlpp11/mysql/connection_config.h"
#include "sqlpp11/mysql/connection.h"
#include "connection_pool.h"
#include <string>
#include <chrono>

#include <asio.hpp>

struct Test
{
	std::mutex _test_mutex;
	asio::io_service& _io_service;
public:
	Test(asio::io_service& io_service)
		: _io_service(io_service)
	{}
	template<typename Bind>
	void post(Bind func, Bind callback)
	{
		_io_service.post(
			[this, callback, func]()
		{
			func();
			_io_service.post(callback);
		}
		);
	}
	void PromptInput(int i)
	{
		std::lock_guard<std::mutex> lock(_test_mutex);
		std::thread::id id = std::this_thread::get_id();
		std::stringstream id_text;
		id._To_text(id_text);
		std::cerr << "Prompt input: " << id_text.str() << " " << i << std::endl;
	}

	void PrintResult(int i)
	{
		std::lock_guard<std::mutex> lock(_test_mutex);
		std::thread::id id = std::this_thread::get_id();
		std::stringstream id_text;
		id._To_text(id_text);
		std::cerr << "Result is in: " << id_text.str() << " " << i << std::endl;
	}

};

struct thread_pool
{
	std::vector<std::thread> threads;
	asio::io_service& _io_service;

	thread_pool(asio::io_service& io_service, unsigned int thread_count)
		: _io_service(io_service)
	{
		for (unsigned i = 0; i < thread_count; i++)
		{
			threads.push_back(std::move(std::thread([&] {io_service.run(); })));
		}
	}
	~thread_pool()
	{
		_io_service.stop();
	}
};

int main()
{
	namespace sql = sqlpp::mysql;
	auto config = std::make_shared<sql::connection_config>();
	config->user     = "root";
	config->password = "password";
	config->database = "sqlpp_mysql";
	config->debug    = true;

	sqlpp::connection_pool<sql::connection, sql::connection_config> pool(config, 4);
	auto conn = pool.get_connection();
	auto conn1 = pool.get_connection();
	auto conn2 = pool.get_connection();
	auto conn3 = pool.get_connection();
	auto conn4 = pool.get_connection();
	pool.free_connection(std::move(conn));
	pool.free_connection(std::move(conn1));
	pool.free_connection(std::move(conn2));
	pool.free_connection(std::move(conn3));
	pool.free_connection(std::move(conn4));
	auto conn5 = pool.get_connection();

	std::thread::id id = std::this_thread::get_id();
	std::stringstream id_text;
	id._To_text(id_text);
	std::cerr << "Current thread: " << id_text.str() << std::endl;

	asio::io_service io_service;
	Test test(io_service);
	test.post(std::bind(&Test::PromptInput, &test, 1), std::bind(&Test::PrintResult, &test, 1));
	test.post(std::bind(&Test::PromptInput, &test, 2), std::bind(&Test::PrintResult, &test, 2));
	test.post(std::bind(&Test::PromptInput, &test, 3), std::bind(&Test::PrintResult, &test, 3));

	auto t_pool = new thread_pool(io_service, 4);
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2s);
	delete t_pool;

	return 0;
}