#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(GCMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // The row with the settings / Geode / Globed buttons
        auto bottomMenu = this->getChildByID("bottom-menu");
        if (!bottomMenu) return true;

        auto spr = CCSprite::create("gc_button.png"_spr);
        if (!spr) return true;

        // Scale to the size of the other bottom menu icons
        spr->setScale(50.f / spr->getContentHeight());

        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(GCMenuLayer::onChess)
        );
        btn->setID("gc-button"_spr);

        bottomMenu->addChild(btn);
        bottomMenu->updateLayout();

        return true;
    }

    void onChess(CCObject*) {
        // Placeholder until the chess layer exists
        FLAlertLayer::create("GDChess", "Chess is coming soon!", "OK")->show();
    }
};
