#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScreenshotsPopup : public geode::Popup<> {
protected:
    bool setup() override;
    std::vector<std::filesystem::path> getScreenshotPaths();
    void onScreenshotClicked(CCObject*);

public:
    static ScreenshotsPopup* create();
};
