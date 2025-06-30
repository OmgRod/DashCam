#pragma once

#include <Geode/Geode.hpp>
#include "ScreenshotsListPopup.hpp"

using namespace geode::prelude;

class ScreenshotPopup : public geode::Popup<std::tuple<int, std::filesystem::path, std::string>, ScreenshotsListPopup*>, SetTextPopupDelegate {
private:
    std::tuple<int, std::filesystem::path, std::string> m_screenshotData;
    ScreenshotsListPopup* m_parentPopup;
    LazySprite* m_screenshot;

protected:
    bool setup(std::tuple<int, std::filesystem::path, std::string>, ScreenshotsListPopup*) override;
    void deleteImage(CCObject*);
    void editImage(CCObject*);
    void openImage(CCObject*);
    void setTextPopupClosed(SetTextPopup*, std::string) override;

public:
    static ScreenshotPopup* create(std::tuple<int, std::filesystem::path, std::string>, ScreenshotsListPopup*);
};
