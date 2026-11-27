#include "ChessMenu.hpp"
#include "GameHistory.hpp"

#include <ctime>
#include <random>
#include <string>
#include <vector>

// TEMP: shows fake rows while there are no real games, so you can check the
// layout and colors. Set to false once the board saves real games.
static constexpr bool SHOW_SAMPLE_GAMES = true;

namespace {
    const ccColor3B kWhite{255, 255, 255};
    const ccColor3B kGrey{140, 140, 140};
    const ccColor3B kGreen{90, 220, 110};
    const ccColor3B kRed{235, 75, 75};
    const ccColor3B kBlack{0, 0, 0};

    std::string getUsername() {
        std::string name = GJAccountManager::get()->m_username;
        if (name.empty()) name = GameManager::get()->m_playerName;
        if (name.empty()) name = "Player";
        return name;
    }

    std::string pickMotd(std::string const& user) {
        std::vector<std::string> motds;

        auto json = utils::file::readJson(Mod::get()->getResourcesDir() / "motd.json");
        if (json.isOk()) {
            auto arr = json.unwrap()["motds"].asArray();
            if (arr.isOk()) {
                for (auto const& m : arr.unwrap()) {
                    auto s = m.asString();
                    if (s.isOk()) motds.push_back(s.unwrap());
                }
            }
        }
        if (motds.empty()) motds.push_back("Ready for a game?");

        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<size_t> dist(0, motds.size() - 1);
        std::string motd = motds[dist(rng)];

        size_t pos = 0;
        while ((pos = motd.find("{user}", pos)) != std::string::npos) {
            motd.replace(pos, 6, user);
            pos += user.size();
        }
        return motd;
    }

    std::string formatDate(int64_t t) {
        std::time_t tt = static_cast<std::time_t>(t);
        std::tm* tmv = std::localtime(&tt);
        if (!tmv) return "-";
        char buf[32];
        std::strftime(buf, sizeof(buf), "%b %d", tmv);
        return buf;
    }

    void addCell(CCNode* parent, std::string const& text, float x, float y,
                 float anchorX, ccColor3B color, float maxW) {
        auto label = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
        label->setAnchorPoint({anchorX, 0.5f});
        label->setPosition({x, y});
        label->setColor(color);
        label->limitLabelWidth(maxW, 0.6f, 0.2f);
        parent->addChild(label);
    }

    // Same columns for the header and every row
    void addColumns(CCNode* row, float w, float h,
                    std::string const& user, std::string const& elo,
                    std::string const& result, std::string const& change,
                    std::string const& date,
                    ccColor3B userC, ccColor3B eloC, ccColor3B resultC,
                    ccColor3B changeC, ccColor3B dateC) {
        float y = h / 2.f;
        addCell(row, user,   w * 0.03f, y, 0.f,  userC,   w * 0.34f);
        addCell(row, elo,    w * 0.46f, y, 0.5f, eloC,    w * 0.12f);
        addCell(row, result, w * 0.60f, y, 0.5f, resultC, w * 0.13f);
        addCell(row, change, w * 0.76f, y, 0.5f, changeC, w * 0.13f);
        addCell(row, date,   w * 0.92f, y, 0.5f, dateC,   w * 0.14f);
    }

    CCNode* makeRow(GameRecord const& g, float w, float h, bool striped) {
        auto row = CCNode::create();
        row->setContentSize({w, h});

        if (striped) {
            row->addChild(CCLayerColor::create({255, 255, 255, 12}, w, h));
        }

        ccColor3B resultC = g.result == "Win" ? kGreen : (g.result == "Loss" ? kRed : kWhite);
        ccColor3B changeC = g.eloChange > 0 ? kGreen : (g.eloChange < 0 ? kRed : kWhite);
        std::string change = g.eloChange > 0
            ? "+" + std::to_string(g.eloChange)
            : std::to_string(g.eloChange);

        addColumns(row, w, h, g.username, std::to_string(g.elo), g.result, change,
                   formatDate(g.date), kWhite, kWhite, resultC, changeC, kGrey);
        return row;
    }

    std::vector<GameRecord> sampleGames() {
        auto now = static_cast<int64_t>(std::time(nullptr));
        constexpr int64_t day = 86400;
        // oldest first, like the real history
        return {
            {"Fischer", 2785, "Loss", -14, now - 5 * day},
            {"Tal",     2705, "Win",   11, now - 4 * day},
            {"Hikaru",  2790, "Win",    9, now - 2 * day},
            {"Magnus",  2830, "Loss", -12, now - 1 * day},
        };
    }
}

ChessMenu* ChessMenu::create() {
    auto ret = new ChessMenu();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCScene* ChessMenu::scene() {
    auto scene = CCScene::create();
    scene->addChild(ChessMenu::create());
    return scene;
}

bool ChessMenu::init() {
    if (!CCLayer::init()) return false;
    this->setKeypadEnabled(true);

    auto win = CCDirector::sharedDirector()->getWinSize();
    auto user = getUsername();

    // ---- background ----
    this->addChild(CCLayerColor::create({17, 17, 17, 255}, win.width, win.height), -2);

    // ---- MOTD bar (top tab) ----
    constexpr float barH = 34.f;
    auto bar = CCLayerColor::create({28, 28, 28, 255}, win.width, barH);
    bar->setPosition({0.f, win.height - barH});
    this->addChild(bar);

    auto barLine = CCLayerColor::create({255, 255, 255, 255}, win.width, 1.f);
    barLine->setPosition({0.f, win.height - barH});
    this->addChild(barLine);

    auto motd = CCLabelBMFont::create(pickMotd(user).c_str(), "bigFont.fnt");
    motd->limitLabelWidth(win.width - 100.f, 0.55f, 0.2f);
    motd->setPosition({win.width / 2.f, win.height - barH / 2.f});
    this->addChild(motd, 2);

    // ---- layout numbers ----
    float panelW = std::min(330.f, win.width * 0.58f);
    float panelX = win.width - panelW - 15.f;
    float panelTop = win.height - barH - 40.f;
    float panelBottom = 15.f;
    float panelH = panelTop - panelBottom;
    float leftCx = panelX / 2.f;

    // ---- menu: back + play ----
    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});
    this->addChild(menu, 5);

    auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(ChessMenu::onBack));
    backBtn->setPosition({28.f, win.height - barH - 26.f});
    menu->addChild(backBtn);

    auto title = CCLabelBMFont::create("GDChess", "bigFont.fnt");
    title->limitLabelWidth(panelX - 20.f, 1.0f, 0.3f);
    title->setPosition({leftCx, win.height * 0.62f});
    this->addChild(title, 2);

    auto playBg = CCScale9Sprite::create("square02_001.png");
    float btnW = std::min(150.f, panelX - 20.f);
    playBg->setContentSize({btnW, 46.f});
    auto playLabel = CCLabelBMFont::create("Play", "bigFont.fnt");
    playLabel->setColor(kBlack);
    playLabel->setScale(0.8f);
    playLabel->setPosition({btnW / 2.f, 23.f});
    playBg->addChild(playLabel);

    auto playBtn = CCMenuItemSpriteExtra::create(playBg, this, menu_selector(ChessMenu::onPlay));
    playBtn->setPosition({leftCx, win.height * 0.42f});
    menu->addChild(playBtn);

    // ---- recent games panel ----
    auto heading = CCLabelBMFont::create("Recent games", "bigFont.fnt");
    heading->setAnchorPoint({0.f, 0.5f});
    heading->setScale(0.5f);
    heading->setPosition({panelX, panelTop + 18.f});
    this->addChild(heading, 2);

    // 1px white border, then the dark panel on top
    auto border = CCLayerColor::create({255, 255, 255, 255}, panelW + 2.f, panelH + 2.f);
    border->setPosition({panelX - 1.f, panelBottom - 1.f});
    this->addChild(border, 1);

    auto panel = CCLayerColor::create({24, 24, 24, 255}, panelW, panelH);
    panel->setPosition({panelX, panelBottom});
    this->addChild(panel, 2);

    constexpr float headerH = 22.f;
    auto header = CCNode::create();
    header->setContentSize({panelW, headerH});
    header->setPosition({panelX, panelTop - headerH});
    header->addChild(CCLayerColor::create({40, 40, 40, 255}, panelW, headerH));
    addColumns(header, panelW, headerH, "User", "Elo", "Result", "+/-", "Date",
               kGrey, kGrey, kGrey, kGrey, kGrey);
    this->addChild(header, 3);

    // ---- scrolling list (newest first) ----
    auto games = GameHistory::load();
    if (games.empty() && SHOW_SAMPLE_GAMES) games = sampleGames();

    float listH = panelH - headerH;
    constexpr float rowH = 24.f;

    if (games.empty()) {
        auto empty = CCLabelBMFont::create("No games played yet", "chatFont.fnt");
        empty->setColor(kGrey);
        empty->setScale(0.7f);
        empty->setPosition({panelX + panelW / 2.f, panelBottom + listH / 2.f});
        this->addChild(empty, 3);
        return true;
    }

    auto scroll = ScrollLayer::create({panelW, listH});
    scroll->setPosition({panelX, panelBottom});
    this->addChild(scroll, 3);

    int i = 0;
    for (auto it = games.rbegin(); it != games.rend(); ++it, ++i) {
        scroll->m_contentLayer->addChild(makeRow(*it, panelW, rowH, i % 2 == 1));
    }
    scroll->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(listH)
            ->setGap(0.f)
    );
    scroll->m_contentLayer->updateLayout();
    scroll->moveToTop();

    return true;
}

void ChessMenu::onPlay(CCObject*) {
    // Placeholder until the board exists
    FLAlertLayer::create("GDChess", "The board is coming next!", "OK")->show();
}

void ChessMenu::onBack(CCObject*) {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void ChessMenu::keyBackClicked() {
    this->onBack(nullptr);
}
