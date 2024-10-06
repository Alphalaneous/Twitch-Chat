#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScene.hpp>
#include "ChatPanel.hpp"
#include "ChatCell.hpp"

using namespace geode::prelude;

bool ChatPanel::init() {
    if (!CCMenu::init()) return false;
    m_background = CCScale9Sprite::create("square02b_001.png");

    CCSize winSize = CCDirector::get()->getWinSize();
    m_cells = CCArray::create();
    
    m_mainLayer = CCNode::create();

    setContentSize({130, winSize.height});
    setBackground();

    m_dummyItem = CCMenuItem::create(); // hacky way to swallow touch
    m_dummyItem->setContentSize(getContentSize());
    m_dummyItem->setAnchorPoint({0, 0});
    m_dummyItem->ignoreAnchorPointForPosition(true);
    m_mainLayer->setContentSize(getContentSize());
    m_mainLayer->setZOrder(1);

    float scale = 0.98f;

    setScale(scale);
    ignoreAnchorPointForPosition(false);
    setPosition({winSize.width - 3, winSize.height/2});
    setAnchorPoint({1, 0.5});
    setZOrder(1000);
    setVisible(false);

    addChild(m_dummyItem);
    addChild(m_mainLayer);
    instance = this;

    return true;
}

void ChatPanel::setClickThrough(bool clickThrough) {
    m_dummyItem->setVisible(!clickThrough);
}

ChatPanel* ChatPanel::get() {

    CCSize winSize = CCDirector::get()->getWinSize();
    instance->setPosition({winSize.width - 3, winSize.height/2});
    return instance;
}

void ChatPanel::setBackground() {
    if (m_background) removeChild(m_background, true);
    m_background = CCScale9Sprite::create("square02b_001.png");
    m_background->setColor({0, 0, 0});
    m_background->setOpacity(196);
    m_background->setContentSize(getContentSize()*2);
    m_background->setScale(0.5f);
    m_background->setAnchorPoint({0, 0});
    m_background->ignoreAnchorPointForPosition(true);
    addChild(m_background);
}

void ChatPanel::persist() {
    SceneManager::get()->keepAcrossScenes(this);
}

void ChatPanel::addMessage(matjson::Object messageObject) {
    if(m_messages.size() == Mod::get()->getSettingValue<int64_t>("chat-history")){
        m_messages.pop_front();
        if (m_cells.data()->count() > 0) {
            m_cells->removeFirstObject();
        }
    }
    m_messages.push_back(messageObject);

    if (isVisible()) {
        float padding = 7.5;
        ChatCell* chatCell = ChatCell::create(messageObject, getContentWidth() - padding);
        m_cells->addObject(chatCell);
        refresh();
    }
}

void ChatPanel::regenerateCells() {

    float padding = 7.5;
    m_cells->removeAllObjects();

    for (matjson::Object messageObject : m_messages) {
        ChatCell* chatCell = ChatCell::create(messageObject, getContentWidth() - padding);
        m_cells->addObject(chatCell);
    }
    
    refresh();
}

void ChatPanel::setVisible(bool visible) {
    CCMenu::setVisible(visible);
    if (visible) regenerateCells();
}

void ChatPanel::refresh() {
    
    float padding = 7.5;

    m_mainLayer->removeAllChildren();

    CCNode* messageContainer = CCNode::create();
    messageContainer->setContentSize({getContentSize().width - padding, getContentSize().height - padding});
    messageContainer->setPosition({padding/2, padding/2});
    RowLayout* layout = RowLayout::create();
    layout->setAxisAlignment(AxisAlignment::Start);
    layout->setAutoScale(false);
    layout->setAxisReverse(true);
    layout->setCrossAxisOverflow(true);
    layout->setGrowCrossAxis(true);
    layout->setGap(0);

    messageContainer->setLayout(layout);

    for (CCNode* cell : CCArrayExt<CCNode*>(m_cells)) {
        messageContainer->addChild(cell);
    }

    messageContainer->updateLayout();

    float height = messageContainer->getContentSize().height + 7.5f;
    if (height < getContentSize().height) {
        height = getContentSize().height;
    }

    ScrollLayer* scrollLayer = ScrollLayer::create({getContentSize().width, height}, true);
	scrollLayer->setContentSize(getContentSize());   
    scrollLayer->m_contentLayer->addChild(messageContainer);
    m_mainLayer->addChild(scrollLayer);

    m_scrollLayer = scrollLayer;
    getYourPrioritiesStraight();
}

void ChatPanel::getYourPrioritiesStraight() {
    if (m_scrollLayer) {
        if (auto delegate = typeinfo_cast<CCTouchDelegate*>(m_scrollLayer)) {
            if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
                CCTouchDispatcher::get()->setPriority(-512, handler->getDelegate());
            }
        }
        if (auto delegate = typeinfo_cast<CCMouseDelegate*>(m_scrollLayer)) {
            CCDirector::get()->getMouseDispatcher()->forceRemoveDelegate(delegate);
        }
    }
    if (auto delegate = typeinfo_cast<CCTouchDelegate*>(this)) {
        if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
            CCTouchDispatcher::get()->setPriority(-511, handler->getDelegate());
        }
    }
}

ChatPanel* ChatPanel::create() {
    auto ret = new ChatPanel;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

class $modify(CCDirector) {

    void willSwitchToScene(CCScene* scene) {
        CCDirector::willSwitchToScene(scene);
        if (ChatPanel* panel = ChatPanel::get()) {
            panel->getYourPrioritiesStraight();
            panel->setBackground();
        }
    }
};

class $modify(CCMouseDispatcher) {

    bool dispatchScrollMSG(float y, float x) {
        if (ChatPanel* panel = ChatPanel::get()) {
            if (panel->isVisible() && panel->m_scrollLayer) {
                
                CCPoint mousePos = getMousePos();
                if (panel->boundingBox().containsPoint(mousePos)) {
                    panel->m_scrollLayer->scrollWheel(y, x);
                    return true;
                }

            }
        }
        return CCMouseDispatcher::dispatchScrollMSG(y, x);
    }
};

class $modify(CCScene) {

    int getHighestChildZ() {
        if (ChatPanel* panel = ChatPanel::get()) {
            auto z = panel->getZOrder();
            panel->setZOrder(0);
            auto ret = CCScene::getHighestChildZ();
            panel->setZOrder(z);
            return ret;
        }
        
        return CCScene::getHighestChildZ();
    }
};


class $modify(PlayLayer) {

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        if (ChatPanel* panel = ChatPanel::get()) {
            panel->setClickThrough(true);
        }
        return true;
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (ChatPanel* panel = ChatPanel::get()) {
            panel->setClickThrough(false);
        }
    }

    void onExit() {
        PlayLayer::onExit();
        if (ChatPanel* panel = ChatPanel::get()) {
            panel->setClickThrough(false);
        }
    }

    void resume() {
        PlayLayer::resume();
        if (ChatPanel* panel = ChatPanel::get()) {
            panel->setClickThrough(true);
        }
    }

    void pauseGame(bool p0){
        PlayLayer::pauseGame(p0);
        if (ChatPanel* panel = ChatPanel::get()) {
            panel->setClickThrough(false);
        }
    }
};