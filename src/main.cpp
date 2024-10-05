#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ServerListener.hpp"
#include "ChatPanel.hpp"
#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

bool madeChatPanel = false;

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