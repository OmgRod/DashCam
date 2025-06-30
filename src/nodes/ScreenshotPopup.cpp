#include "ScreenshotPopup.hpp"
#include "../Utils.hpp"
#include "ScreenshotsListPopup.hpp"

using namespace geode::prelude;
using namespace dashcam;

bool ScreenshotPopup::setup(std::tuple<int, std::filesystem::path, std::string> screenshotData, ScreenshotsListPopup* parentPopup) {
    m_screenshotData = screenshotData;
    m_parentPopup = parentPopup;

    this->setTitle(std::get<2>(m_screenshotData));

    auto tempScreenshot = CCSprite::create(std::get<1>(m_screenshotData).string().c_str());
    auto screenshotSize = tempScreenshot->getContentSize();

    m_screenshot = LazySprite::create(screenshotSize, true);

    float maxWidth = m_mainLayer->getContentSize().width * 0.8f;
    float maxHeight = m_mainLayer->getContentSize().height * 0.6f;
    auto size = m_screenshot->getContentSize();
    float scaleX = maxWidth / size.width;
    float scaleY = maxHeight / size.height;

    tempScreenshot->release();

    float scale = std::min(std::min(scaleX, scaleY), 1.0f);

    m_screenshot->setScale(scale);
    m_screenshot->setPosition(m_mainLayer->getContentSize() / 2);
    m_screenshot->loadFromFile(std::get<1>(m_screenshotData).string().c_str());
    m_mainLayer->addChild(m_screenshot);

    auto openImageButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Open Image"),
        this,
        menu_selector(ScreenshotPopup::openImage)
    );
    openImageButton->setPosition({ m_mainLayer->getContentSize().width * 0.5f, m_mainLayer->getContentSize().height * 0.1f });
    m_buttonMenu->addChild(openImageButton);

    auto menuLayout = SimpleAxisLayout::create(Axis::Column);
    menuLayout->setGap(2.5f)
        ->setCrossAxisAlignment(CrossAxisAlignment::Center)
        ->setCrossAxisDirection(AxisDirection::TopToBottom)
        ->setMainAxisScaling(AxisScaling::Fit);

    auto menu = CCMenu::create();
    menu->setContentSize({ 40.f, m_mainLayer->getContentSize().height * 0.6f });
    menu->setPosition({ 30.f, m_mainLayer->getContentSize().height * 0.5f });
    menu->setAnchorPoint({ 0.5f, 0.5f });
    menu->setLayout(menuLayout);
    m_mainLayer->addChild(menu);

    auto editIcon = CCSprite::create("edit.png"_spr);
    auto editSprite = CircleButtonSprite::create(
        editIcon,
        CircleBaseColor::Green,
        CircleBaseSize::Medium
    );
    editIcon->setScale(1.f);
    editSprite->setScale(41.f / 46.75f);
    auto editButton = CCMenuItemSpriteExtra::create(
        editSprite,
        this,
        menu_selector(ScreenshotPopup::editImage)
    );
    menu->addChild(editButton);

    auto deleteSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    auto deleteButton = CCMenuItemSpriteExtra::create(
        deleteSprite,
        this,
        menu_selector(ScreenshotPopup::deleteImage)
    );
    menu->addChild(deleteButton);

    menu->updateLayout();

    return true;
}

ScreenshotPopup* ScreenshotPopup::create(std::tuple<int, std::filesystem::path, std::string> screenshotData, ScreenshotsListPopup* parentPopup) {
    auto ret = new ScreenshotPopup();
    if (ret->initAnchored(380.f, 230.f, screenshotData, parentPopup, "GJ_square01.png")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

void ScreenshotPopup::deleteImage(CCObject*) {
    createQuickPopup("Warning", "Are you sure you want to <cr>delete</c> this screenshot? <cr>This action is irreversible.</c>", "No", "Yes", [this](auto, bool btn2){
        if (btn2) {
            std::error_code ec;
            std::filesystem::remove(std::get<1>(m_screenshotData), ec);
            if (ec) {
                FLAlertLayer::create("Error", fmt::format("Failed to delete the screenshot file: {}", ec), "OK")->show();
            } else {
                onClose(nullptr);
                m_parentPopup->refresh(nullptr);
            }
        }
    });
}

void ScreenshotPopup::setTextPopupClosed(SetTextPopup* popup, std::string text) {
    auto oldPath = std::get<1>(m_screenshotData);
    auto filename = text;
    if (!filename.ends_with(".png")) {
        filename = fmt::format("{}.png", filename);
    }
    auto newPath = oldPath.parent_path() / filename;
    std::error_code ec;
    std::filesystem::rename(oldPath, newPath, ec);
    if (ec) {
        FLAlertLayer::create("Error", fmt::format("Failed to rename the screenshot file: {}", ec), "OK")->show();
        return;
    }

    onClose(nullptr);

    if (m_parentPopup) {
        m_parentPopup->refresh(nullptr);
    }
}

void ScreenshotPopup::editImage(CCObject*) {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    auto popup = SetTextPopup::create(
        std::get<2>(m_screenshotData),
        "Enter name here...",
        30,
        "Rename Screenshot",
        "Done",
        true,
        1.f
    );
    popup->m_delegate = this;
    popup->m_scene = scene;

    popup->show();
    popup->setZOrder(scene->getHighestChildZ() + 1);
}

void ScreenshotPopup::openImage(CCObject*) {
    file::openFolder(std::get<1>(m_screenshotData));
}