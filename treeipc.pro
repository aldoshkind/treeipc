QT += core

TEMPLATE = app
CONFIG += console c++11

SOURCES += main.cpp \
    client_node.cpp \
    server.cpp \
    client.cpp \
    package.cpp \
    io_service.cpp \
    socket_client.cpp \
    ../tree/filepath_utils.cpp \
    ../tree/property_listener.cpp \
    ../tree/resource.cpp \
    ../tree/tree_node.cpp \
    ../tree/tree_node_inherited.cpp \
    property_fake.cpp \
    proxy_node_generator.cpp \
    node_sync.cpp

HEADERS += \
    device.h \
    server.h \
    client.h \
    package.h \
    client_node.h \
    property_serializer.h \
    socket_device.h \
    io_service.h \
    acceptor.h \
    conn_server.h \
    ../tree/filepath_utils.h \
    ../tree/property_listener.h \
    ../tree/resource.h \
    ../tree/tree_node.h \
    property_fake.h \
    proxy_node_factory_base.h

INCLUDEPATH += ../ ../tree

LIBS += -pthread -lboost_system
