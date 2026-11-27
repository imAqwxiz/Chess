#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ChessMenu : public CCLayer {
protected:
    bool init() override;
    void keyBackClicked() override;
    void onBack(CCObject*);
    void onPlay(CCObject*);

public:
    static ChessMenu* create();
    static CCScene* scene();
};
