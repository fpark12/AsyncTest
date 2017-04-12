#include <memory>
#include <iostream>
#include <string>
#include <chrono>
#include <future>

#include <asio.hpp>

#include <mysql.h>

#include <sqlpp11/sqlpp11.h>
#include "sqlpp11/mysql/connection_config.h"
#include "sqlpp11/mysql/connection.h"
#include "connection_handle.h"
#include "connection_pool.h"
#include "thread_pool.h"
#include "query_task.h"
#include "sqlpp11/bind.h"

#include "async_query_service.h"
#include "SampleTable.h"

int main()
{
	namespace sql = sqlpp::mysql;
	auto config = std::make_shared<sqlpp::mysql::connection_config>();
	config->user = "root";
	config->password = "password";
	config->database = "sqlpp_mysql";
	config->debug = true;

	sqlpp::connection_pool_t<sqlpp::mysql::connection_config, sqlpp::connection_validator::automatic> pool(config, 4);
	sqlpp::connection_pool_t<sqlpp::mysql::connection_config> pool1(config, 4);
	sqlpp::connection_pool_t<sqlpp::mysql::connection_config, sqlpp::connection_validator::periodic> pool2(config, 4);
	auto pool3 = sqlpp::connection_pool/*<sqlpp::mysql::connection_config, sqlpp::mysql::connection,
		sqlpp::connection_validator::automatic>*/(config, 4);
	auto pool4 = sqlpp::connection_pool(config, 4);
	auto pool5 = sqlpp::connection_pool<sqlpp::mysql::connection_config, sqlpp::connection_validator::none>(config, 4);
	// For connectors that are not up to date:
	auto pool6 = sqlpp::connection_pool<sqlpp::mysql::connection_config, sqlpp::connection_validator::automatic, sqlpp::mysql::connection>(config, 4);

	auto conn = sqlpp::mysql::connection(config);
	auto conn1 = pool.get_connection();
	auto conn2 = pool.get_connection();
	auto conn3 = pool.get_connection();
	auto conn4 = pool.get_connection();
	auto conn5 = pool.get_connection();

	//auto conn = sql::connection(config);
	//auto rcp = sqlpp::connection_validator::automatic();
	//rcp.deregister();

	//*
	std::thread::id id = std::this_thread::get_id();
	std::stringstream id_text;
	id._To_text(id_text);
	std::cerr << "Current thread: " << id_text.str() << std::endl;

	asio::io_service io_service;
	//sqlpp::async_query_service<asio::io_service> async_query_service(io_service, 4);
	const auto Users = SQLTable::Users{};
	auto query = select(all_of(Users)).from(Users).unconditionally();
	auto query2 = select(all_of(Users)).from(Users).unconditionally();

	conn1.execute(R"(DROP TABLE IF EXISTS Users)");
	conn1.execute(R"(CREATE TABLE Users (
			ID bigint AUTO_INCREMENT,
			AccountName varchar(255) NOT NULL,
			Password varchar(255) NOT NULL,
			EmailAddress varchar(255) NOT NULL,
			PRIMARY KEY (ID)
			))");

	// Prepared insert
	auto p1 = conn1.prepare(
		insert_into(Users).set(
			Users.AccountName = parameter(Users.AccountName),
			Users.Password = parameter(Users.Password),
			Users.EmailAddress = parameter(Users.EmailAddress)
		));

	struct UserInfo
	{
		std::string AccountName, Password, EmailAddress;
	};

	UserInfo input_values[] =
	{
		{"yellowtail", "loveSush1"  , "jellyfish3311@gmail.com"},
		{"tuna"      , "loveSash1m1", "paperfish@gmail.com"    },
		{"salmon"    , "deliciousme", "peppercorn11@gmail.com" },
	};

	for (const auto& input : input_values)
	{
		//prepared_insert.params.alpha = input.first;
		p1.params.AccountName = input.AccountName;
		p1.params.Password = input.Password;
		p1.params.EmailAddress = input.EmailAddress;
		conn1(p1);
	}

	for (const auto& row : conn1(query))
	{
		if (row.AccountName.is_null())
			std::cerr << "AccountName is null" << std::endl;
		else
			std::string AccountName = row.AccountName;   // string-like fields are implicitly convertible to string
	}

	/*
	std::packaged_task<int()> task([]() { return 7; }); // wrap the function
	std::packaged_task<decltype(conn1(query))()> task2([&]() { return conn1(query); }); // wrap the function
	std::future<int> f1 = task.get_future();
	std::future<decltype(conn1(query))> f2 = task2.get_future();
	*/
	/*
	_io_service._impl.async_query([&]()
	{
		auto async_connection = connection_pool_t.get_connection();
		using result_type = decltype(async_connection(query));
		std::packaged_task<result_type()> task([&]() { return async_connection(query); }); // wrap the function
		std::future<decltype(async_connection(query))> future = task.get_future();
		callback(async_connection(query));
	}
	);
	//*/


	// ok
	auto callback = [&](auto error, auto result, auto conn)
	{
		if (error)
		{
			std::cerr << error.what() << std::endl;
		}

		try
		{
			for (const auto& row : result)
			{
				if (row.AccountName.is_null())
					std::cerr << "AccountName is null" << std::endl;
				else
					std::string AccountName = row.AccountName;
			}
		}
		catch (sqlpp::exception e)
		{
			// result processing error
		}
	};

	// ok
	auto callback2 = [&](auto error, auto result)
	{
		for (const auto& row : result)
		{
			if (row.AccountName.is_null())
				std::cerr << "AccountName is null" << std::endl;
			else
				std::string AccountName = row.AccountName;
		}
	};

	// ok
	auto callback3 = [&](auto error)
	{
	};

	// ok
	auto callback4 = [&]()
	{
	};

	using Connection_pool = decltype(pool);
	using Query = decltype(query);
	using Lambda = decltype(callback);
  auto b = sqlpp::bind(pool, query, callback);
  auto c = sqlpp::bind(std::move(conn), query, callback);
  sqlpp::bind(pool, query, callback)();
  sqlpp::bind(pool.get_connection(), query, callback)();

  sqlpp::bind(std::move(conn), query, callback)();
  std::async(std::launch::async, b);
  sqlpp::async(b);
  //sqlpp::async(c);
	pool(query);
	auto s1 = dynamic_select(conn);
	auto s2 = dynamic_select(conn1);
	//auto s3 = dynamic_select(pool);
	//auto s4 = dynamic_select(query);

	sqlpp::async(pool, query, callback2);

	thread_pool tpool(4);
	auto task = sqlpp::bind(pool, query, callback);
	tpool.enqueue(task);

	/*
	async_query_service.async_query(pool, query, callback);
	async_query_service.async_query(pool, query, callback2);
	async_query_service.async_query(pool, query, callback3);
	async_query_service.async_query(pool, query, callback4);
	*/
	/*
	async_query_service.async_query(pool, query, [&](auto result, auto conn)
	{
		for (const auto& row : result)
		{
			if (row.AccountName.is_null())
				std::cerr << "AccountName is null" << std::endl;
			else
				std::string AccountName = row.AccountName;   // string-like fields are implicitly convertible to string
		}
	});
	*/
	//async_query_service.async_query(pool, query, std::bind(&sqlpp::async_query_service::FuncB, &async_query_service, 3));

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(10s);
	//*/
	return 0;
}

/*

void FuncA(int i)
{
std::lock_guard<std::mutex> lock(_test_mutex);
std::thread::id id = std::this_thread::get_id();
std::stringstream id_ss;
id._To_text(id_ss);
std::cerr << "FuncA called by thread: " << id_ss.str() << " " << i << std::endl;
}

void FuncB(int i)
{
std::lock_guard<std::mutex> lock(_test_mutex);
std::thread::id id = std::this_thread::get_id();
std::stringstream id_ss;
id._To_text(id_ss);
std::cerr << "FuncB called by thread: " << id_ss.str() << " " << i << std::endl;
}
*/