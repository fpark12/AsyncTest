SET( DEFINE
)
SET( INCLUDE
boost2
)
SET( LINK
boost2
C:/boost_1_63_0/bin.v2/libs/system/build/msvc-14.0/debug/link-static/threading-multi/libboost_system-vc140-mt-gd-1_63.lib
C:/boost_1_63_0/bin.v2/libs/date_time/build/msvc-14.0/debug/link-static/threading-multi/libboost_date_time-vc140-mt-gd-1_63.lib
C:/boost_1_63_0/bin.v2/libs/regex/build/msvc-14.0/debug/link-static/threading-multi/libboost_regex-vc140-mt-gd-1_63.lib
C:/boost_1_63_0/bin.v2/libs/context/build/msvc-14.0/debug/link-static/threading-multi/libboost_context-vc140-mt-gd-1_63.lib
C:/boost_1_63_0/bin.v2/libs/coroutine/build/msvc-14.0/debug/link-static/threading-multi/libboost_coroutine-vc140-mt-gd-1_63.lib
)

create_project(CONSOLE DEFINE INCLUDE LINK)
