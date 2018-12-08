cmake_minimum_required(VERSION 3.0)

project (logflux)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions("/wd4267")

# logflux
add_executable(logflux WIN32  
clipboard.cpp
config.cpp
loglistview.cpp
logserver.cpp
mainframe.cpp
search_list_dialog.cpp
socket.cpp
toolbarctrlex.cpp
udpserver.cpp
util.cpp
utils/inifile.cpp
utils/systemutils.cpp
logflux.cpp
logflux.rc
)

target_link_libraries(logflux tinyxml2)
