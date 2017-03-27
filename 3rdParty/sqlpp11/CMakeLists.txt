SET( DEFINE
)
SET( INCLUDE
3rdparty/boost/preprocessor
co2
libmariadb
hinnantdate
)
SET( LINK
co2
libmariadb
hinnantdate
)
create_project(STATIC DEFINE INCLUDE LINK)
