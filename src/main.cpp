#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCTextInputNode.hpp>
#include <Geode/modify/FLAlertLayer.hpp>
#include "ServerListener.hpp"
#include "ChatPanel.hpp"
#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

bool madeChatPanel = false;

class ModSettingsPopup {};
class StringSettingNodeV3 {};

bool bypassText = false;

class $modify(FLAlertLayer) {
    
    void show() {
        bypassText = false;
        if (typeinfo_cast<ModSettingsPopup*>(this)) {
            if (CCLayer* layer = getChildOfType<CCLayer>(this, 0)) {
                if (CCLabelBMFont* title = getChildOfType<CCLabelBMFont>(layer, 0)) {
                    if (std::string_view(title->getString()) == "Settings for Twitch Chat") {
                        bypassText = true;
                    }
                }
            }
        }
        FLAlertLayer::show();
    }
};

class $modify(CCTextInputNode) {

    void updateLabel(gd::string p0) {
        if (bypassText) {
            CCNode* parent = this;
            while (true) {
                if (!typeinfo_cast<StringSettingNodeV3*>(parent)) {
                    parent = parent->getParent();
                    if (!parent) break;
                }
                else {
                    setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
                    break;
                }
            }
        }
        CCTextInputNode::updateLabel(p0);
    }
};

class $modify(MenuLayer) {

    bool init() {
        if (!MenuLayer::init()) return false;

        if (!madeChatPanel) {
            auto chatPanel = ChatPanel::create();
            chatPanel->setID("chat-panel"_spr);
            chatPanel->persist();
            ServerListener::get()->connectAsync();

            new EventListener([=](InvokeBindEvent* event) {
                if (ChatPanel* panel = ChatPanel::get()) {
                    auto pushToView = Mod::get()->getSettingValue<bool>("push-to-view");
                    if (event->isDown()) {
                        
                        if (pushToView) {
                            panel->setVisible(true);
                            if (panel->isVisible()) panel->refresh();
                        }
                        else {
                            panel->setVisible(!panel->isVisible());
                        }
                    }
                    else {
                        if (pushToView) {
                            panel->setVisible(false);
                        }
                    }
                }
                return ListenerResult::Propagate;
            }, InvokeBindFilter(nullptr, "show-twitch-chat"_spr));
            madeChatPanel = true;
        }

        return true;
    }
};

$execute {

    listenForSettingChanges("twitch-username", [](std::string value) {
        queueInMainThread([] {
            ChatPanel::get()->removeFromParent();
            bool visible = ChatPanel::get()->isVisible();
            SceneManager::get()->forget(ChatPanel::get());
            auto chatPanel = ChatPanel::create();
            chatPanel->setID("chat-panel"_spr);
            chatPanel->persist();
            chatPanel->setVisible(visible);
            ServerListener::get()->connectToChannel();
        });
    });
    
    BindManager::get()->registerBindable({
        "show-twitch-chat"_spr,
        "Show Twich Chat",
        "Shows Twitch Chat, change whether it is push to show or toggle in the mod settings",
        { Keybind::create(KEY_T, Modifier::None) },
        "Twitch Chat",
        false
    });
}