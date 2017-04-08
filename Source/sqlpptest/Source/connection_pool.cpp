#include <ciso646>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_set>
#include <mutex>
#include "connection_pool.h"
#include <sqlpp11/connection.h>
#include <sqlpp11/exception.h>

/*
SQLConnectionPool::SQLConnectionPool() :
ActiveConnectionCount(0)
{
	ConnectionPoolInfo.ConnectionCount = SQLConnectionPoolInfo::InvalidConnectionPool;
}

SQLConnectionPool::SQLConnectionPool(SQLConnectionPoolInfo& info) :
	ActiveConnectionCount(0),
	ConnectionPoolInfo(info)
{

}

error SQLConnectionPool::AddPreparedStatementString(uint32 statementStringIndex, char * statementString)
{
	//TODO: Error checking
	PreparedStatementStrings[statementStringIndex] = statementString;
	return error::ok;
}

error SQLConnectionPool::SpawnConnections()
{
	if (ConnectionPoolInfo.ConnectionCount <= 0)
	{
		return error::ok;
	}
	if (ActiveConnectionCount != ConnectionPoolInfo.ConnectionCount)
	{
		Connections.reserve(ConnectionPoolInfo.ConnectionCount);
		for (int i = ActiveConnectionCount; i < ConnectionPoolInfo.ConnectionCount; ++i)
		{
			Connections.emplace_back(SQLConnection(ConnectionPoolInfo));
			if (error::ok != Connections[i].Connect())
			{
				//TODO Error handling
				return error::failed;
			}
		
			if (error::ok != Connections[i].InitPreparedStatements(PreparedStatementStrings))
			{
				//TODO Error handling
				return error::failed;
			}
		}
		ActiveConnectionCount = ConnectionPoolInfo.ConnectionCount;
	}
	
	return error::ok;
}

SQLConnection* SQLConnectionPool::GetAvaliableSQLConnection()
{
	bool Expected = true;
	for (auto& conn : Connections)
	{
		Expected = true;
		if (conn.IsAvaliable.compare_exchange_strong(Expected, false))
		{
			return &conn;
		}
	}
	return nullptr;
}

*/