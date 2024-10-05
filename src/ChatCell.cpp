#include <Geode/Geode.hpp>
#include "ChatCell.hpp"
#include "ChatPanel.hpp"
#include "ImageCache.hpp"

using namespace geode::prelude;

static std::unordered_map<std::string, std::string> badges = {
    {"staff", "d97c37bd-a6f5-4c38-8f57-4e4bef88af34"},
    {"partner", "d12a2e27-16f6-41d0-ab77-b780518f00a3"},
    {"premium", "bbbe0db0-a598-423e-86d0-f9fb98ca1933"},
    {"broadcaster", "5527c58c-fb7d-422d-b71b-f309dcb85cc1"},
    {"moderator", "3267646d-33f0-4b17-b3df-f923a41db1d0"},
    {"vip", "b817aba4-fad8-49e2-b88a-7cc744dfa6ec"},
    {"founder", "511b78a9-ab37-472f-9569-457753bbe7d3"},
    {"artist-badge", "4300a897-03dc-4e83-8c0e-c332fee7057f"},
    {"no_audio", "aef2cd08-f29b-45a1-8c12-d44d7fd5e6f0"},
    {"no_video", "199a0dba-58f3-494e-a7fc-1fa0a1001fb8"},
    {"subscriber", "5d9f2208-5dd8-11e7-8513-2ff4adfae661"}
};

static std::array<ccColor3B, 15> chatColors = {
    ccColor3B{255, 0, 0},
    ccColor3B{0, 0, 255},
    ccColor3B{0, 128, 0},
    ccColor3B{178, 34, 34},
    ccColor3B{255, 127, 80},
    ccColor3B{154, 205, 50},
    ccColor3B{255, 69, 0},
    ccColor3B{46, 139, 87},
    ccColor3B{218, 165, 32},
    ccColor3B{210, 105, 30},
    ccColor3B{95, 158, 160},
    ccColor3B{30, 144, 255},
    ccColor3B{255, 105, 180},
    ccColor3B{138, 43, 226},
    ccColor3B{0, 255, 127}
};

int random(int min, int max) {
    static bool first = true;
    if (first) {  
        srand(time(NULL));
        first = false;
    }
    return min + rand() % (( max + 1 ) - min);
}

bool ChatCell::init(matjson::Object messageObject, float width) {
    if (!CCNode::init()) return false;
    
    m_mainLayer = CCNode::create(); 
    m_mainLayer->setContentSize({width, 20});

    RowLayout* layout = RowLayout::create();
    layout->setAxisAlignment(AxisAlignment::Start);
    layout->setCrossAxisAlignment(AxisAlignment::End);
    layout->setGrowCrossAxis(true);
    layout->setAutoScale(false);
    layout->setGap(0);
    m_mainLayer->setLayout(layout);

    std::string username = "unknown";
    std::string message = "unknown";
    ccColor3B usernameColor = {58, 77, 199};

    if (messageObject.contains("message")) {
        message = messageObject["message"].as_string();
    }

    std::vector<EmoteInfo> emotes;
    std::vector<std::string> messageParts = utils::string::split(message, " ");

    CCNode* dummyNode = CCNode::create();
    dummyNode->setContentSize({0, 10});
    m_mainLayer->addChild(dummyNode);

    if (messageObject.contains("username")) {
        username = messageObject["username"].as_string();
    }
    if (messageObject.contains("tags")) {
        matjson::Object tags = messageObject["tags"].as_object();
        if (tags.contains("display-name")) {
            username = tags["display-name"].as_string();
        }
        if (tags.contains("color")) {
            std::string colorStr = tags["color"].as_string();
            Result<ccColor3B> colorRes = cc3bFromHexString(colorStr);
            if (colorRes.isOk()) {
                usernameColor = colorRes.unwrap();
            }
        }
        else {
            if (!ChatPanel::get()->m_defaultColorUsers.contains(username)) {
                ChatPanel::get()->m_defaultColorUsers[username] = chatColors[random(0, chatColors.size()-1)];
            }
            usernameColor = ChatPanel::get()->m_defaultColorUsers[username];
        }
        if (tags.contains("badges")) {
            std::string badgeStr = tags["badges"].as_string();
            std::vector<std::string> badgeParts = utils::string::split(badgeStr, ",");

            for (std::string badge : badgeParts) {
                std::vector<std::string> badgeAttr = utils::string::split(badge, "/");
                if (badges.contains(badgeAttr.at(0))) {
                    std::string badgeUrl = fmt::format("https://static-cdn.jtvnw.net/badges/v1/{}/2", utils::string::toLower(badges[badgeAttr.at(0)]));
                    CCNode* badgeNode = getImage(badgeUrl, fmt::format("badge:{}", badges[badgeAttr.at(0)]), 0.8f);
                    m_mainLayer->addChild(badgeNode);
                }
            }
        }
        if (tags.contains("emotes")) {
            std::string emotesStr = tags["emotes"].as_string();

            std::vector<std::string> emoteParts = utils::string::split(emotesStr, "/");

            for (std::string part : emoteParts) {
                std::vector<std::string> separated = utils::string::split(part, ":");
                std::string id = separated.at(0);
                std::string range = separated.at(1);

                std::vector<std::string> ranges = utils::string::split(range, ",");

                for (std::string rangeVal : ranges) {

                    std::vector<std::string> rangeParts = utils::string::split(rangeVal, "-");
                    Result<int> rangeStartRes = utils::numFromString<int>(rangeParts.at(0));
                    Result<int> rangeEndRes = utils::numFromString<int>(rangeParts.at(1));
                    int start = -1;
                    int end = -1;

                    if (rangeStartRes.isOk()) {
                        start = rangeStartRes.unwrap();
                    }
                    if (rangeEndRes.isOk()) {
                        end = rangeEndRes.unwrap();
                    }
                    std::string subStr = message.substr(start, end-start + 1);
                    int i = 0;
                    for (std::string msgPart : messageParts) {
                        if (subStr == msgPart) {
                            bool found = false;
                            for (EmoteInfo emote : emotes) {
                                if (emote.idx == i) {
                                    found = true; 
                                    break;
                                }
                            }
                            if (found) {
                                i++;
                                continue;
                            }
                            EmoteInfo info = EmoteInfo{id, i};
                            emotes.push_back(info);
                            break;
                        }
                        i++;
                    }
                }
            }
        }
    }

    CCNode* usernameNode = CCNode::create();

    CCLabelBMFont* usernameLabel = createLabel(fmt::format("{}", username));
    usernameLabel->setColor(usernameColor);
    usernameLabel->setAnchorPoint({0, 0});
    CCLabelBMFont* usernameLabel2 = createLabel(fmt::format("{}", username));
    usernameLabel2->setColor(usernameColor);
    usernameLabel2->setAnchorPoint({0, 0});
    usernameLabel2->setPositionX(0.2f);
    usernameNode->setContentSize(usernameLabel->getScaledContentSize());

    usernameNode->addChild(usernameLabel);
    usernameNode->addChild(usernameLabel2);

    m_mainLayer->addChild(usernameNode);


    CCLabelBMFont* colonLabel = createLabel(": ");
    m_mainLayer->addChild(colonLabel);

    int idx = 0;
    for (std::string part : messageParts) {

        bool found = false;
        EmoteInfo emoteResult;
        for (EmoteInfo emote : emotes) {
            if (emote.idx == idx) {
                found = true; 
                emoteResult = emote;
                break;
            }
        }
        if (found) {
            CCNode* emoteNode = getImage(fmt::format("https://static-cdn.jtvnw.net/emoticons/v2/{}/static/dark/2.0", emoteResult.id), fmt::format("emote:{}", emoteResult.id));
            m_mainLayer->addChild(emoteNode);
            CCLabelBMFont* spaceLabel = createLabel(" ");
            m_mainLayer->addChild(spaceLabel);
        }
        else {
            CCLabelBMFont* partLabel = createLabel(fmt::format("{} ", part));
            if (partLabel->getScaledContentSize().width > m_mainLayer->getContentWidth()) {
                for (unsigned i = 0; i < part.length(); i += 35) {
                    std::string newPart = part.substr(i, 35);
                    CCLabelBMFont* newPartLabel = createLabel(fmt::format("{}", newPart));
                    m_mainLayer->addChild(newPartLabel);
                }
            }
            else m_mainLayer->addChild(partLabel);
        }
        idx++;
    }

    m_mainLayer->updateLayout();

    addChild(m_mainLayer);

    float height = m_mainLayer->getContentHeight();

    setContentSize({m_mainLayer->getContentWidth(), height});
    m_mainLayer->ignoreAnchorPointForPosition(true);

    return true;
}

CCLabelBMFont* ChatCell::createLabel(std::string text) {
    CCLabelBMFont* label = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
    label->setScale(0.4f);

    return label;
}

CCNode* ChatCell::getImage(std::string url, std::string id, float scale) {
    CCNode* container = CCNode::create();
    container->setContentSize(CCSize{10, 10});
    container->retain();
    ImageCache::get()->downloadImage(url, id, [container, scale](CCImage* img, bool success) {
        if (success) {
            CCSprite* spr;
            CCTexture2D* texture = new CCTexture2D();
            if (texture->initWithImage(img)) {
                spr = CCSprite::createWithTexture(texture);
            }
            texture->release();
            if (spr) {
                spr->setPosition(CCSize{5 * scale, 5});

                float relScale = container->getContentHeight() / spr->getContentHeight();
                spr->setScale(relScale * scale);
                container->addChild(spr);
            }
        }
        container->release();
    }); 

    return container;
}

ChatCell* ChatCell::create(matjson::Object messageObject, float width) {
    auto ret = new ChatCell;
    if (ret->init(messageObject, width)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}