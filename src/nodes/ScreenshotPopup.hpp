#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScreenshotPopup : public geode::Popup<std::tuple<int, std::filesystem::path, std::string>> {
protected:
    bool setup(std::tuple<int, std::filesystem::path, std::string>) override;
    std::vector<std::filesystem::path> getScreenshotPaths();
    void onScreenshotClicked(CCObject*);

public:
    static ScreenshotPopup* create(std::tuple<int, std::filesystem::path, std::string>);
};
