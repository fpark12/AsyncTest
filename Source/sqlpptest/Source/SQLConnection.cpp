//#include "SQLConnection.h"
/*
SQLConnection::SQLConnection() : IsAvaliable(true)
{

}

SQLConnection::SQLConnection(SQLConnectionInfo& _info) :
	MySqlHandle(nullptr),
	IsAvaliable(true),
	ConnectionInfo(_info)
{
}

SQLConnection::~SQLConnection()
{
	//TODO free up all the prepared statements
	if (MySqlHandle)
	{
		mysql_close(MySqlHandle);
	}
}

error SQLConnection::Connect()
{
	MYSQL* initMysql = mysql_init(NULL);
	if (!initMysql)
	{
		// TODO Error Log: not enough memory to spawn a new mysql connection handle
		return error::failed;
	}

	mysql_options(initMysql, MYSQL_SET_CHARSET_NAME, "utf8");

	MySqlHandle = mysql_real_connect(initMysql, ConnectionInfo.Hostname.c_str(),
		ConnectionInfo.Username.c_str(), ConnectionInfo.Password.c_str(),
		ConnectionInfo.Schema.c_str(), ConnectionInfo.Port, NULL, 0);

	if (!MySqlHandle)
	{
		// TODO Error Log: unsuccess message TC_LOG_ERROR("sql.sql", "Could not connect to MySQL database at %s: %s", m_connectionInfo.host.c_str(), mysql_error(mysqlInit));
		// mysql_errno(mysqlInit)
		uint32 error_code = mysql_errno(initMysql);
		mysql_print_error(initMysql);
		mysql_close(initMysql);
		return error::failed;
	}

	// TODO Logger: Get library & server version info
	// TC_LOG_INFO("sql.sql", "MySQL client library: %s", mysql_get_client_info());
	// TC_LOG_INFO("sql.sql", "MySQL server ver: %s ", mysql_get_server_info(m_Mysql));

	// TODO Logger: Success message TC_LOG_INFO("sql.sql", "Connected to MySQL database at %s", m_connectionInfo.host.c_str());

	mysql_autocommit(MySqlHandle, 1); //Turn on auto commit mode

	mysql_set_character_set(MySqlHandle, "utf8");
	return error::ok;
}

error SQLConnection::InitPreparedStatements(std::vector<char*>& preparedStmtStrings)
{
	for (auto& stmt : preparedStmtStrings)
	{
		if (error::ok != PrepareStatement(stmt))
		{
			//TODO Error Handling
		}
	}
	return error::ok;
}

error SQLConnection::PrepareStatement(char* stmtString)
{
	MYSQL_STMT* MySQLStatement = mysql_stmt_init(MySqlHandle);
	if (!MySQLStatement)
	{
		//TODO Error handling
		return error::failed;
	}

	if (mysql_stmt_prepare(MySQLStatement, stmtString, static_cast<unsigned long>(strlen(stmtString))))
	{
		//TODO Error handling
		return error::failed;
	}

	PreparedStatements.push_back(MySQLStatement);
	return error::ok;
}

*/