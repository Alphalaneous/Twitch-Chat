#pragma once

#ifndef __SERVERLISTENER_H
#define __SERVERLISTENER_H

#include <Geode/Geode.hpp>

using namespace cocos2d;

class ServerListener {
protected:
    static ServerListener* instance;

public:

    void connectAsync();
    void connect();
    void open();
    void close();
    void onMessage(std::string message);
    void connectToChannel();
    static ServerListener* get(){

        if (!instance) {
            instance = new ServerListener();
        };
        return instance;
    }

private:
};

#endif