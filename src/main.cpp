#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ChessMenu.hpp"

using namespace geode::prelude;

class $modify(GCMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto bottomMenu = this->getChildByID("bottom-menu");
        if (!bottomMenu) return true;

        auto spr = CCSprite::create("gc_button.png"_spr);
        if (!spr) return true;

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
        CCDirector::sharedDirector()->pushScene(
            CCTransitionFade::create(0.5f, ChessMenu::scene())
        );
    }
};
