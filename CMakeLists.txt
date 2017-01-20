cmake_minimum_required( VERSION 3.0 )

project( GtkLuaConsole )

# Required packages
find_package( Lua51 REQUIRED )
find_package( GTK2 2.10 REQUIRED gtk )

add_executable( console
    main.cpp
)

include_directories(
    ${LUA_INCLUDE_DIR}
    ${GTK2_INCLUDE_DIRS}
)

target_link_libraries( console
    ${GTK2_LIBRARIES}
    ${LUA_LIBRARIES}
)

