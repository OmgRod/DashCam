#include "ScreenshotsListPopup.hpp"
#include "ScreenshotPopup.hpp"
#include "../Utils.hpp"

using namespace geode::prelude;
using namespace dashcam;

bool ScreenshotsListPopup::setup() {
    this->setTitle("Screenshots");

    float scrollWidth = m_mainLayer->getContentWidth() * 0.8f;
    float scrollHeight = m_mainLayer->getContentHeight() * 0.75f;
    float padX = scrollWidth * 0.02f;
    float padY = scrollHeight * 0.02f;

    auto scroll = ScrollLayer::create({ scrollWidth, scrollHeight }, true, true);
    auto border = Border::create(scroll, {115, 63, 38, 0}, {scrollWidth, scrollHeight}, {padX, padY});
    border->setPosition({ m_mainLayer->getContentWidth() * 0.1f, m_mainLayer->getContentHeight() * 0.125f });
    m_mainLayer->addChild(border);

    float layoutWidth = scrollWidth - padX * 2.f;
    float layoutHeight = scrollHeight - padY * 2.f;

    auto scrollLayout = ColumnLayout::create();
    float rowGap = layoutWidth * 0.03f;
    scrollLayout->setGap(rowGap)
        ->setAutoScale(false)
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::Center);
    scroll->m_contentLayer->setLayout(scrollLayout);

    const int maxPerRow = 3;
    float rowPadding = layoutWidth * 0.03f;
    float maxSpriteWidth = layoutWidth * 0.34f;
    float maxSpriteHeight = layoutHeight * 0.3f;

    auto screenshots = getScreenshotPaths();

    int count = 0;
    CCMenu* row = nullptr;
    std::vector<CCMenuItemSpriteExtra*> rowItems;
    std::vector<std::tuple<int, std::filesystem::path, std::string>> screenshotData;

    float maxContentWidth = 0.f;

    for (size_t i = 0; i < screenshots.size(); ++i) {
        if (count % maxPerRow == 0) {
            if (row && !rowItems.empty()) {
                float maxHeight = 0.f;
                float rowWidth = 0.f;

                for (auto item : rowItems) {
                    auto size = item->getScaledContentSize();
                    maxHeight = std::max(maxHeight, size.height);
                    rowWidth += size.width;
                }

                if (!rowItems.empty()) rowWidth += rowPadding * (rowItems.size() - 1);
                maxContentWidth = std::max(maxContentWidth, rowWidth);

                row->setContentSize({ rowWidth, maxHeight });
                row->updateLayout();
                rowItems.clear();
            }

            row = CCMenu::create();
            auto rowLayout = RowLayout::create();
            rowLayout->setGap(rowPadding)
                ->setAutoScale(false)
                ->setAxisAlignment(AxisAlignment::Center);
            row->setLayout(rowLayout);
            scroll->m_contentLayer->addChild(row);
        }

        auto sprite = CCSprite::create(screenshots[i].string().c_str());
        if (!sprite) continue;

        auto spriteSize = sprite->getContentSize();
        float scaleX = maxSpriteWidth / spriteSize.width;
        float scaleY = maxSpriteHeight / spriteSize.height;
        float scale = std::min({ scaleX, scaleY, 1.f });
        sprite->setScale(scale);

        auto item = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(ScreenshotsListPopup::onScreenshotClicked)
        );
        item->setTag(static_cast<int>(i));
        item->setAnchorPoint({ 0.5f, 0.5f });
        item->ignoreAnchorPointForPosition(true);
        row->addChild(item);
        rowItems.push_back(item);

        screenshotData.emplace_back(
            static_cast<int>(i),
            screenshots[i],
            screenshots[i].filename().string()
        );

        ++count;
    }

    if (row && !rowItems.empty()) {
        float maxHeight = 0.f;
        float rowWidth = 0.f;

        for (auto item : rowItems) {
            auto size = item->getScaledContentSize();
            maxHeight = std::max(maxHeight, size.height);
            rowWidth += size.width;
        }

        if (!rowItems.empty()) rowWidth += rowPadding * (rowItems.size() - 1);
        maxContentWidth = std::max(maxContentWidth, rowWidth);

        row->setContentSize({ rowWidth, maxHeight });
        row->updateLayout();
        rowItems.clear();
    }

    float totalHeight = 0.f;
    auto children = scroll->m_contentLayer->getChildren();
    for (size_t i = 0; i < children->count(); ++i) {
        auto row = static_cast<CCNode*>(children->objectAtIndex(i));
        totalHeight += row->getContentSize().height;
        if (i > 0) totalHeight += rowGap;
    }

    scroll->m_contentLayer->setContentHeight(totalHeight);
    scroll->m_contentLayer->setContentWidth(std::max(layoutWidth, maxContentWidth));
    scroll->m_contentLayer->updateLayout();
    scroll->scrollToTop();

    Utils::shared()->setScreenshots(screenshotData);

    return true;
}

void ScreenshotsListPopup::onScreenshotClicked(CCObject* sender) {
    auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
    int tag = item->getTag();

    const auto& screenshots = Utils::shared()->getScreenshots();
    for (const auto& tup : screenshots) {
        if (std::get<0>(tup) == tag) {
            ScreenshotPopup::create(tup)->show();
            break;
        }
    }
}

std::vector<std::filesystem::path> ScreenshotsListPopup::getScreenshotPaths() {
    auto directory = std::filesystem::path(Mod::get()->getSaveDir() / "screenshots");
    std::vector<std::filesystem::path> paths;

    if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                paths.push_back(entry.path());
            }
        }
        std::sort(paths.begin(), paths.end());
    }

    return paths;
}

ScreenshotsListPopup* ScreenshotsListPopup::create() {
    auto ret = new ScreenshotsListPopup();
    if (ret->initAnchored(440.f, 270.f, "GJ_square01.png")) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}
