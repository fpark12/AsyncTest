#include <memory>
#include <iostream>
#include <string>
#include <chrono>

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
	config->user     = "root";
	config->password = "password";
	config->database = "sqlpp_mysql";
	config->debug    = true;

	sqlpp::connection_pool<sqlpp::mysql::connection, sqlpp::mysql::connection_config> pool(config, 4);
	auto conn1 = pool.get_connection();
	auto conn2 = pool.get_connection();
	auto conn3 = pool.get_connection();
	auto conn4 = pool.get_connection();
	auto conn5 = pool.get_connection();

	//auto conn = sql::connection(config);
	//auto rcp = sqlpp::reconnect_policy::auto_reconnect();
	//rcp.clean();

	//*
	std::thread::id id = std::this_thread::get_id();
	std::stringstream id_text;
	id._To_text(id_text);
	std::cerr << "Current thread: " << id_text.str() << std::endl;

	asio::io_service io_service;
	sqlpp::async_query_service test(io_service, 4);
	const auto tab = SampleTable{};
	auto query = select(all_of(tab)).from(tab).unconditionally();

	conn1(select(all_of(tab)).from(tab).unconditionally());

	test.post(pool, query, std::bind(&sqlpp::async_query_service::FuncB, &test, 1));
	test.post(pool, query, std::bind(&sqlpp::async_query_service::FuncB, &test, 2));
	test.post(pool, query, std::bind(&sqlpp::async_query_service::FuncB, &test, 3));

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2s);
	//*/
	return 0;
}