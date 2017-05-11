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

#include "SampleTable.h"

#include <sqlpp11/coroutine.h>

void test1()
{
  auto config = std::make_shared<sqlpp::mysql::connection_config>();
  config->user = "root";
  config->password = "password";
  config->database = "sqlpp_mysql";
  config->debug = true;

  sqlpp::connection_pool<sqlpp::mysql::connection> pool(config, 4);
  auto pool2 = sqlpp::make_connection_pool(config, 4);

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
  /*
  auto b = sqlpp::bind2(pool, query, callback);
  auto c = sqlpp::bind2(std::move(conn), query, callback);
  sqlpp::bind2(pool, query, callback)();
  sqlpp::bind2(pool.get_connection(), query, callback)();
  */
  //sqlpp::bind(std::move(conn), query, callback)();
  //sqlpp::async(b);
  //sqlpp::async(c);
  //pool(query);
  //pool(query, callback);
  //conn1(query, callback);
  /*
  conn1(query, callback2);
  conn1(query, callback3);
  conn1(query, callback4);
  */
  auto s1 = dynamic_select(conn);
  auto s2 = dynamic_select(conn1);
  //auto s3 = dynamic_select(pool);
  //auto s4 = dynamic_select(query);



}

std::string thread_id()
{
  auto id = std::this_thread::get_id();
  std::stringstream id_text;
  id._To_text(id_text);
  return id_text.str();
}

std::shared_ptr<sqlpp::connection_pool<sqlpp::mysql::connection>> connection_pool;
SQLTable::Users users;

static std::mutex coro_mutex;

resumable_function test2()
{
  auto& pool = *connection_pool.get();
  auto query = select(all_of(users)).from(users).unconditionally();

  auto callback = [&](auto& future)
  {
    std::lock_guard<std::mutex> lock(coro_mutex);
    std::cerr << "Callback, thread # " << thread_id() << std::endl;

    try
    {
      auto& connection = future.get_connection();

      for(const auto& row : future.get_result())
      {
        if (row.AccountName.is_null())
          std::cerr << "AccountName is null" << std::endl;
        else
          std::cerr << "AccountName: " << row.AccountName << std::endl;
      }
    }
    catch(const std::exception& e)
    {
      std::cout << "Caught exception " << e.what() << '\n';
    }
  };

  // sqlpp::async allocates task state on heap and enqueues immediately.
  sqlpp::async(pool, query); // discard future
  sqlpp::async(pool, query, callback); // discard future
  sqlpp::async(pool, query, callback);

  //*
  // sqlpp::deferred allocates task state on heap without enqueuing.
  auto f1 = sqlpp::deferred(pool, query, callback);
  auto f2 = sqlpp::deferred(pool, query);
  // let co_await enqueue the task
  co_await f1;
  co_await f2;

  try
  {
    std::lock_guard<std::mutex> lock(coro_mutex);
    std::cerr << "Continuation, thread # " << thread_id() << std::endl;
    auto& conn = f2.get_connection();

    for (const auto& row : f2.get_result())
    {
      if (row.AccountName.is_null())
        std::cerr << "AccountName is null" << std::endl;
      else
        std::cerr << "AccountName: " << row.AccountName << std::endl;
    }
  }
  catch (sqlpp::exception e)
  {
    std::cerr << e.what() << std::endl;
  }

  co_return;
}

int main()
{
  /*
  std::async(std::launch::async, []
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    return 3; });
    */

  namespace sql = sqlpp::mysql;
  auto config = std::make_shared<sqlpp::mysql::connection_config>();
  config->user = "root";
  config->password = "password";
  config->database = "sqlpp_mysql";
  config->debug = false;

  connection_pool = std::make_shared<sqlpp::connection_pool<sqlpp::mysql::connection>>(config, 4);
  const auto users = SQLTable::Users{};

  auto coro = test2();

  //auto task = sqlpp::bind(pool, query, callback);
  /*
  tpool.enqueue(task);

  auto task2 = [] { std::this_thread::sleep_for(std::chrono::milliseconds(3000)); };

  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  tpool.enqueue(task2);
  */
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
  while (true)
  {
    static int i = 0;
    i++;
  }

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1000s);
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