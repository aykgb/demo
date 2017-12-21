cmake_minimum_required(VERSION 2.8.9)
project(libevent_demo)

add_definitions(
    --std=c++11
    -g -O0
    -W -Wall
    )

include_directories(${PROJECT_SOURCE_DIR})

link_directories(
    /usr/lib
    /usr/local/lib
    /ira.cao/marcelo/opt/boost/lib
    )

link_libraries(
    pthread
    boost_regex
    boost_thread
    boost_system
    event
    )

add_executable( echo_server echo_server.cc )
add_executable( echo_client echo_client.cc )
add_executable( epoll_server epoll_server.cc )
add_executable( epoll_client epoll_client.cc )
add_executable( epoll_reactor epoll_reactor.cc )
add_executable( ev_timer ev_timer.cc )
add_executable( unixdomain_serv unixdomain_serv.cc )
add_executable( unixdomain_cli unixdomain_cli.cc )
