set(SRC
    net_io.cpp
    PrivateGreaterThan.cpp
    PPDTServer.cpp
    PPDTClient.cpp
    Timer.cpp)
add_library(ppdt STATIC ${SRC})
