#include <Geode/Geode.hpp>
#include "ChatCell.hpp"
#include "ChatPanel.hpp"

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

std::vector<MessagePiece> parseMessagePieces(const ChatMessage& chatMsg) {
    std::vector<MessagePiece> result;

    const std::string& msg = chatMsg.getMessage();
    size_t pos = 0, index = 0;

    struct EmoteRangeRef {
        Range range;
        const EmoteInfo* info;
    };
    std::vector<EmoteRangeRef> emoteRanges;

    for (const auto& emote : chatMsg.getEmotes()) {
        for (const auto& range : emote.getRanges()) {
            emoteRanges.push_back({ range, &emote });
        }
    }

    std::sort(emoteRanges.begin(), emoteRanges.end(), [](const EmoteRangeRef& a, const EmoteRangeRef& b) {
        return a.range.start < b.range.start;
    });

    while (pos < msg.length()) {
        size_t next = msg.find(' ', pos);
        if (next == std::string::npos) next = msg.length();

        Range wordRange = { static_cast<int>(pos), static_cast<int>(next - 1) };
        bool matchedEmote = false;

        for (const auto& em : emoteRanges) {
            if (wordRange.start == em.range.start && wordRange.end == em.range.end) {
                result.push_back(MessagePiece{
                    .isEmote = true,
                    .string = msg.substr(pos, next - pos),
                    .emote = *em.info
                });
                matchedEmote = true;
                break;
            }
        }

        if (!matchedEmote) {
            result.push_back(MessagePiece{
                .isEmote = false,
                .string = msg.substr(pos, next - pos),
                .emote = {}
            });
        }
        pos = next + 1;
    }
    return result;
}

bool ChatCell::init(ChatMessage chatMessage, float width) {
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

    ccColor3B usernameColor = {58, 77, 199};
    if (chatMessage.getHasColor()) usernameColor = chatMessage.getColor();
    else {
        if (!ChatPanel::get()->m_defaultColorUsers.contains(chatMessage.getDisplayName())) {
            ChatPanel::get()->m_defaultColorUsers[chatMessage.getDisplayName()] = chatColors[random(0, chatColors.size()-1)];
        }
        usernameColor = ChatPanel::get()->m_defaultColorUsers[chatMessage.getDisplayName()];
    }

    std::vector<std::string> messageParts = utils::string::split(chatMessage.getDisplayName(), " ");

    CCNode* dummyNode = CCNode::create();
    dummyNode->setContentSize({0, 10});
    m_mainLayer->addChild(dummyNode);

    for (const BadgeInfo& badge : chatMessage.getBadges()) {
        if (badges.contains(badge.getName())) {
            std::string badgeUrl = fmt::format("https://static-cdn.jtvnw.net/badges/v1/{}/2", utils::string::toLower(badges[badge.getName()]));
            CCNode* badgeNode = getImage(badgeUrl, fmt::format("badge:{}", badges[badge.getName()]), 0.8f);
            m_mainLayer->addChild(badgeNode);
        }
    }

    CCNode* usernameNode = CCNode::create();

    CCLabelBMFont* usernameLabel = createLabel(fmt::format("{}", chatMessage.getDisplayName()));
    usernameLabel->setColor(usernameColor);
    usernameLabel->setAnchorPoint({0, 0});
    CCLabelBMFont* usernameLabel2 = createLabel(fmt::format("{}", chatMessage.getDisplayName()));
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

    std::vector<MessagePiece> pieces = parseMessagePieces(chatMessage);

    for (const MessagePiece& piece : pieces) {
        if (piece.isEmote) {
            CCNode* emoteNode = getImage(fmt::format("https://static-cdn.jtvnw.net/emoticons/v2/{}/static/dark/2.0", piece.emote.getID()), fmt::format("emote:{}", piece.emote.getID()));
            m_mainLayer->addChild(emoteNode);
            CCLabelBMFont* spaceLabel = createLabel(" ");
            m_mainLayer->addChild(spaceLabel);
        }
        else {
            std::string part = piece.string;
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
    LazySprite* lazySpr = LazySprite::create(CCSize{5*scale, 5});
    lazySpr->setPosition(container->getContentSize()/2);
    lazySpr->setLoadCallback([lazySpr, scale, container](Result<> res) {
        if (res.isOk()) {
            float relScale = container->getContentHeight() / lazySpr->getContentHeight();
            lazySpr->setScale(relScale * scale);
        }
    });
    lazySpr->setAutoResize(true);
    lazySpr->loadFromUrl(url);
    container->addChild(lazySpr);
    return container;
}

ChatCell* ChatCell::create(ChatMessage chatMessage, float width) {
    auto ret = new ChatCell();
    if (ret->init(chatMessage, width)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}