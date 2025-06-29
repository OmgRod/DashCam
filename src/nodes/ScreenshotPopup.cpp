#include "ScreenshotPopup.hpp"
#include "../Utils.hpp"

using namespace geode::prelude;
using namespace dashcam;

bool ScreenshotPopup::setup(std::tuple<int, std::filesystem::path, std::string> screenshotData) {
    this->setTitle(std::get<2>(screenshotData));

    return true;
}

ScreenshotPopup* ScreenshotPopup::create(std::tuple<int, std::filesystem::path, std::string> screenshotData) {
    auto ret = new ScreenshotPopup();
    if (ret->initAnchored(380.f, 230.f, screenshotData, "GJ_square01.png")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}
