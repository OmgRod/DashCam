#ifndef GEODE_IS_IOS

#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include "Utils.hpp"

using namespace geode::prelude;
using namespace keybinds;
using namespace dashcam;

bool screenshot = false;
bool recording = false;

$execute {
    BindManager::get()->registerBindable({
        "screenshot"_spr,
        "Screenshot",
        "Screenshots your current frame in-game. You can access the screenshots later in the mod's settings.",
        { Keybind::create(KEY_F6, Modifier::Control) },
        "DashCam"
    });

	BindManager::get()->registerBindable({
        "record"_spr,
        "Record",
        "Records your current gameplay. Stop recording by pressing Ctrl+F7 again.",
        { Keybind::create(KEY_F7, Modifier::Control) },
        "DashCam"
    });

	new EventListener([=](InvokeBindEvent* event) {
        if (event->isDown()) {
            if (!screenshot) {
                if (!Utils::shared()->screenshot()) {
                    Notification::create("Unable to take screenshot.", NotificationIcon::Error)->show();
                } else {
                    Notification::create("Screenshot successfully taken!", NotificationIcon::Success)->show();
                }
            }
            screenshot = true;
            return ListenerResult::Stop;
        } else {
            screenshot = false;
        }
        return ListenerResult::Propagate;
    }, InvokeBindFilter(nullptr, "screenshot"_spr));

    new EventListener([=](InvokeBindEvent* event) {
        if (event->isDown()) {
            if (!recording) {
                if (!Utils::shared()->isRecording()) {
                    if (!Utils::shared()->startRecording()) {
                        Notification::create("Unable to start recording video.", NotificationIcon::Error)->show();
                    }
                } else {
                    if (!Utils::shared()->stopRecording()) {
                        Notification::create("Unable to save video.", NotificationIcon::Error)->show();
                    } else {
                        Notification::create("Video successfully taken!", NotificationIcon::Success)->show();
                    }
                }
            }
            recording = true;
            return ListenerResult::Stop;
        } else {
            recording = false;
        }
        return ListenerResult::Propagate;
    }, InvokeBindFilter(nullptr, "record"_spr));
}

#endif
