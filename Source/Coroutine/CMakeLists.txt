SET( DEFINE
)
SET( INCLUDE
)
SET( LINK
)

create_project(CONSOLE DEFINE INCLUDE LINK)
if(MSVC)
	add_definitions(/await)
endif()