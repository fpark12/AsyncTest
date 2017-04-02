SET( DEFINE
)
SET( INCLUDE
co2
asio
boost
sqlpp11
sqlpp11-connector-mysql
libmariadb
)
SET( LINK
co2
asio
boost
sqlpp11
sqlpp11-connector-mysql
libmariadb
)

create_project(CONSOLE DEFINE INCLUDE LINK)
