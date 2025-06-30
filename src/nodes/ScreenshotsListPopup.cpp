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
    
    m_scroll = ScrollLayer::create({ scrollWidth, scrollHeight }, true, true);
    auto border = Border::create(m_scroll, {115, 63, 38, 0}, {scrollWidth, scrollHeight}, {padX, padY});
    border->setPosition({ m_mainLayer->getContentWidth() * 0.1f, m_mainLayer->getContentHeight() * 0.125f });
    m_scrollContent = m_scroll->m_contentLayer;
    m_mainLayer->addChild(border);
    
    float layoutWidth = scrollWidth - padX * 2.f;
    float layoutHeight = scrollHeight - padY * 2.f;

    auto scrollLayout = ColumnLayout::create();
    float rowGap = layoutWidth * 0.03f;
    scrollLayout->setGap(rowGap)
        ->setAutoScale(false)
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::Center);
    m_scroll->m_contentLayer->setLayout(scrollLayout);

    refresh(nullptr);

    return true;
}

void ScreenshotsListPopup::onScreenshotClicked(CCObject* sender) {
    auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
    int tag = item->getTag();

    const auto& screenshots = Utils::shared()->getScreenshots();
    for (const auto& tup : screenshots) {
        if (std::get<0>(tup) == tag) {
            ScreenshotPopup::create(tup, this)->show();
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

void ScreenshotsListPopup::refresh(CCObject*) {
    float scrollWidth = m_mainLayer->getContentWidth() * 0.8f;
    float scrollHeight = m_mainLayer->getContentHeight() * 0.75f;
    float padX = scrollWidth * 0.02f;
    float padY = scrollHeight * 0.02f;
    float layoutWidth = scrollWidth - padX * 2.f;
    float layoutHeight = scrollHeight - padY * 2.f;

    float rowGap = layoutWidth * 0.03f;

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

    m_scrollContent->removeAllChildrenWithCleanup(true);

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
            m_scrollContent->addChild(row);
        }

        auto sprite = LazySprite::create({ maxSpriteWidth, maxSpriteHeight });
        sprite->setAutoResize(true);

        sprite->setLoadCallback([sprite, maxSpriteWidth, maxSpriteHeight](Result<> res) {
            if (res) {
                auto size = sprite->getContentSize();
                float scaleX = maxSpriteWidth / size.width;
                float scaleY = maxSpriteHeight / size.height;
                float scale = std::min({ scaleX, scaleY, 1.f });
                sprite->setScale(scale);
            } else {
                log::error("Failed to load screenshot: {}", res.unwrapErr());
                sprite->initWithSpriteFrameName("exMark_001.png");
            }
        });

        sprite->loadFromFile(screenshots[i]);

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
    auto children = m_scrollContent->getChildren();
    for (size_t i = 0; i < children->count(); ++i) {
        auto row = static_cast<CCNode*>(children->objectAtIndex(i));
        totalHeight += row->getContentSize().height;
        if (i > 0) totalHeight += rowGap;
    }

    m_scrollContent->setContentHeight(totalHeight);
    m_scrollContent->setContentWidth(std::max(layoutWidth, maxContentWidth));
    m_scrollContent->updateLayout();
    m_scroll->scrollToTop();

    Utils::shared()->setScreenshots(screenshotData);
}