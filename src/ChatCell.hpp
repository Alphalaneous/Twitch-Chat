#pragma once

#include <Geode/Geode.hpp>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

struct MessagePiece {
    bool isEmote;
    std::string string;
    EmoteInfo emote;
};

class ChatCell : public cocos2d::CCNode {
public:
    static ChatCell* create(ChatMessage chatMessage, float width);
private:
    CCNode* m_mainLayer;
    CCLabelBMFont* createLabel(std::string text);
    bool init(ChatMessage chatMessage, float width);
    CCNode* getImage(std::string url, std::string id, float scale = 1);
};