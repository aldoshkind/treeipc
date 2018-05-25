QT += core

TEMPLATE = app
CONFIG += console c++11

SOURCES += main.cpp \
    tree/filepath_utils.cpp \
    tree/resource.cpp \
    client_node.cpp \
    server.cpp \
    client.cpp \
    package.cpp \
    tree/property_listener.cpp \
    io_service.cpp \
    socket_client.cpp

HEADERS += \
    tree/event_printer.h \
    tree/filepath_utils.h \
    tree/node.h \
    tree/resource.h \
    tree/sinus_generator.h \
    tree/tree_node.h \
    device.h \
    pseudodevice.h \
    server.h \
    client.h \
    package.h \
    client_node.h \
    property_serializer.h \
    socket_device.h \
    tree/property_listener.h \
    io_service.h \
    acceptor.h \
    conn_server.h

LIBS += -pthread -lboost_system
