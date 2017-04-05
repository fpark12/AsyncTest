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

	sqlpp::connection_pool<sqlpp::mysql::connection_config, sqlpp::connection_validator::automatic> pool(config, 4);
	sqlpp::connection_pool<sqlpp::mysql::connection_config> pool1(config, 4);
	sqlpp::connection_pool<sqlpp::mysql::connection_config, sqlpp::connection_validator::periodic> pool2(config, 4);
	auto pool3 = sqlpp::make_connection_pool/*<sqlpp::mysql::connection_config, sqlpp::mysql::connection,
		sqlpp::connection_validator::automatic>*/(config, 4);
	auto pool4 = sqlpp::make_connection_pool(config, 4);
	auto pool5 = sqlpp::make_connection_pool<sqlpp::mysql::connection_config, sqlpp::connection_validator::none>(config, 4);
	// For connectors that are not up to date:
	auto pool6 = sqlpp::make_connection_pool<sqlpp::mysql::connection_config, sqlpp::connection_validator::automatic, sqlpp::mysql::connection>(config, 4);

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
	sqlpp::async_query_service test(io_service, 4);
	const auto Users = SQLTable::Users{};
	auto query = select(all_of(Users)).from(Users).unconditionally();

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

	auto lambda = [](int a) { return 0; };

	std::packaged_task<int()> task([]() { return 7; }); // wrap the function
	std::packaged_task<decltype(conn1(query))()> task2([&]() { return conn1(query); }); // wrap the function
	std::future<int> f1 = task.get_future();
	std::future<decltype(conn1(query))> f2 = task2.get_future();
	/*
	_io_service._impl.async_query([&]()
	{
		auto async_connection = connection_pool.get_connection();
		using result_type = decltype(async_connection(query));
		std::packaged_task<result_type()> task([&]() { return async_connection(query); }); // wrap the function
		std::future<decltype(async_connection(query))> future = task.get_future();
		callback(async_connection(query));
	}
	);
	//*/

	auto callback = [&](auto result)
	{
		for (const auto& row : result)
		{
			if (row.AccountName.is_null())
				std::cerr << "AccountName is null" << std::endl;
			else
				std::string AccountName = row.AccountName;   // string-like fields are implicitly convertible to string
		}
	};
	test.async_query(pool, query, callback);
	test.async_query(pool, query, [&](auto result)
	{
		for (const auto& row : result)
		{
			if (row.AccountName.is_null())
				std::cerr << "AccountName is null" << std::endl;
			else
				std::string AccountName = row.AccountName;   // string-like fields are implicitly convertible to string
		}
	});
	//test.async_query(pool, query, std::bind(&sqlpp::async_query_service::FuncB, &test, 3));

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