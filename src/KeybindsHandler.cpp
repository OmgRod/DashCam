#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/platform/win32/CCGL.h"
#ifndef GEODE_IS_IOS

#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

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
        log::info("Screenshot event triggered");

        auto winSize = CCDirector::sharedDirector()->getWinSizeInPixels();
        int bufferLen = winSize.width * winSize.height * 4;

        unsigned char* buffer = new unsigned char[bufferLen];
        glReadPixels(0, 0, winSize.width, winSize.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            log::warn("glReadPixels error: {}", err);
        }

        unsigned char* flipped = new unsigned char[bufferLen];
        for (int y = 0; y < winSize.height; ++y) {
            for (int x = 0; x < winSize.width; ++x) {
                int srcIndex = (y * winSize.width + x) * 4;
                int dstIndex = ((winSize.height - 1 - y) * winSize.width + x) * 4;
                flipped[dstIndex + 0] = buffer[srcIndex + 0];
                flipped[dstIndex + 1] = buffer[srcIndex + 1];
                flipped[dstIndex + 2] = buffer[srcIndex + 2];
                flipped[dstIndex + 3] = buffer[srcIndex + 3];
            }
        }

        static int iteration = 0;
        static std::time_t lastSecond = 0;

        std::time_t now = std::time(nullptr);
        std::tm local_tm = fmt::localtime(now);

        if (now != lastSecond) {
            lastSecond = now;
            iteration = 0;
        } else {
            iteration++;
        }

        auto screenshotsDir = Mod::get()->getSaveDir() / "screenshots";
        std::filesystem::create_directories(screenshotsDir);

        std::string filename = fmt::format(
            "screenshot_{:%Y-%m-%d_%H-%M-%S}_{}.png",
            local_tm, iteration);

        auto path = screenshotsDir / filename;

        CCImage* image = new CCImage();
        image->initWithImageData(flipped, bufferLen, CCImage::kFmtRawData, winSize.width, winSize.height, 8);
        bool saved = image->saveToFile(path.string().c_str());

        if (!saved) {
            log::warn("Failed to save screenshot to {}", path.string());
        } else {
            log::info("Screenshot saved to {}", path.string());
        }

        delete[] buffer;
        delete[] flipped;
        delete image;

        return ListenerResult::Propagate;
    }, InvokeBindFilter(nullptr, "screenshot"_spr));
}

#endif
