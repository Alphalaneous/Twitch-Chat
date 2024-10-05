#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct EmoteInfo {
    std::string id;
    int idx = -1;
};

class ChatCell : public cocos2d::CCNode {
public:
    static ChatCell* create(matjson::Object messageObject, float width);
private:
    CCNode* m_mainLayer;
    CCLabelBMFont* createLabel(std::string text);
    bool init(matjson::Object messageObject, float width);
    CCNode* getImage(std::string url, std::string id, float scale = 1);
};