
#include "easywsclient.hpp"
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include "ServerListener.hpp"
#include <Geode/Geode.hpp>
#include <thread>
#include "ChatPanel.hpp"

using namespace cocos2d;

using namespace easywsclient;

ServerListener* ServerListener::instance = nullptr;

std::unique_ptr<WebSocket> ws(nullptr);

void ServerListener::connectAsync() {
    std::thread t = std::thread([this] {
        connect();
    });
    t.detach();
}

void ServerListener::connect(){

    open();
    using easywsclient::WebSocket;

#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        return;
    }
#endif

    if (ws) {
        connectToChannel();
        while (ws->getReadyState() != WebSocket::CLOSED) {
            WebSocket::pointer wsp = &*ws; 
            ws->poll();
            ws->dispatch([this, wsp](const std::string& message) {
                onMessage(message);
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    }
    ws.release();

#ifdef _WIN32
    WSACleanup();
#endif

}

void ServerListener::connectToChannel() {
    if (ws) {
        queueInMainThread([this] {
            auto username = Mod::get()->getSettingValue<std::string>("twitch-username");
            ws->send(username);
        });
    }
}

void ServerListener::onMessage(std::string message) {
    queueInMainThread([message] {

        matjson::Value msgValue = matjson::parse(message);
        matjson::Object msgObject = msgValue.as_object();

        ChatPanel::get()->addMessage(msgObject);

        //geode::log::info("{}", message);
    });
}

void ServerListener::close() {
    if (ws) {
        ws.get()->close();
    }
}

void ServerListener::open() {
    ws.reset(WebSocket::from_url("ws://164.152.25.111:6456")); // my twitch reader server
}