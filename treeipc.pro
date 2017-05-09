TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    tree/filepath_utils.cpp \
    tree/resource.cpp

HEADERS += \
    tree/event_printer.h \
    tree/filepath_utils.h \
    tree/node.h \
    tree/resource.h \
    tree/sinus_generator.h \
    tree/tree_node.h \
    tree/widget.h \
    serializer.h
