#pragma once

#include <Geode/Geode.hpp>
#include <deque>
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace geode::prelude;

class ChatPanel : public CCMenu {
public:
    static ChatPanel* create();
    static ChatPanel* get();
    inline static ChatPanel* instance = nullptr;
    std::unordered_map<std::string, ccColor3B> m_defaultColorUsers;
    ScrollLayer* m_scrollLayer = nullptr;
    CCMenuItem* m_dummyItem;
    CCScale9Sprite* m_background;
    void persist();
    void getYourPrioritiesStraight();
    void addMessage(ChatMessage chatMessage);
    void refresh();
    void setClickThrough(bool clickThrough);
    void setBackground();
    void regenerateCells();
    void setVisible(bool) override;

private:
    CCNode* m_mainLayer;
    std::deque<ChatMessage> m_messages;
    Ref<CCArray> m_cells;
    bool init() override;
};