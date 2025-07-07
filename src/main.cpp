#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCTextInputNode.hpp>
#include <Geode/modify/FLAlertLayer.hpp>
#include "ChatPanel.hpp"
#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif
#include <alphalaneous.twitch_chat_api/include/TwitchChatAPI.hpp>

using namespace keybinds;

bool madeChatPanel = false;

class $modify(MenuLayer) {

    bool init() {
        if (!MenuLayer::init()) return false;

        if (!madeChatPanel) {
            auto chatPanel = ChatPanel::create();
            chatPanel->setID("chat-panel"_spr);
            chatPanel->persist();

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
                        TwitchChatAPI::get()->promptLogin();
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
    #ifndef GEODE_IS_IOS
    BindManager::get()->registerBindable({
        "show-twitch-chat"_spr,
        "Show Twich Chat",
        "Shows Twitch Chat, change whether it is push to show or toggle in the mod settings",
        { Keybind::create(KEY_T, Modifier::None) },
        "Twitch Chat",
        false
    });
    #endif

    TwitchChatAPI::get()->registerOnMessageCallback([] (const ChatMessage& chatMessage) {
		ChatPanel::get()->addMessage(chatMessage);
	});
}