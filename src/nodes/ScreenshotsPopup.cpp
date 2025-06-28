#include "ScreenshotsPopup.hpp"

using namespace geode::prelude;

bool ScreenshotsPopup::setup() {
    this->setTitle("Screenshots");

    auto screenshots = getScreenshotPaths();
    for (const auto& path : screenshots) {
        log::info("Screenshot: {}", path.string());
        auto sprite = CCSprite::create(path.string().c_str());
        sprite->setScale(0.3f);
        m_mainLayer->addChild(sprite);
    }
    
    return true;
}

std::vector<std::filesystem::path> ScreenshotsPopup::getScreenshotPaths() {
    auto directory = std::filesystem::path(Mod::get()->getSaveDir() / "screenshots");
    std::vector<std::filesystem::path> paths;
    if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                paths.push_back(entry.path());
            }
        }
    }
    return paths;
}

ScreenshotsPopup* ScreenshotsPopup::create() {
    auto ret = new ScreenshotsPopup();
    if (ret->initAnchored(440.f, 270.f, "GJ_square01.png")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}