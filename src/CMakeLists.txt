set(SRC
    util.cpp
    net_io.cpp
    PrivateGreaterThan.cpp
    PPDTServer.cpp
    PPDTClient.cpp)
add_library(ppdt STATIC ${SRC})
