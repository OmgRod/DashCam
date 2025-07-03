#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScreenshotsListPopup : public geode::Popup<> {
private:
    CCNode* m_scrollContent;
    ScrollLayer* m_scroll;
    Border* m_border;

protected:
    bool setup() override;
    std::vector<std::filesystem::path> getScreenshotPaths();
    void onScreenshotClicked(CCObject*);

public:
    static ScreenshotsListPopup* create();
    void refresh(CCObject*);
    void onClose(CCObject*) override;
};
