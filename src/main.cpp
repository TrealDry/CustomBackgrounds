#include <ctime>
#include <string>
#include <vector>
#include <random>
#include <shellapi.h>  // Only windows os, sorry =(
#include <filesystem>

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;
namespace fs = std::filesystem;


std::random_device rd;
std::mt19937 gen(rd());

bool firstPathInit = true;
std::vector<std::string> backgroundPaths;

CCSprite *bg, *nextBg;
CCSprite *restartBtnSpr;
CircleButtonSprite *openFolderBtnSpr;

CCMenuItemSpriteExtra *restartBtn, *openFolderBtn;

float bgOpacity = 0.f;

fs::path currentPath;
const char* currentBackgroundPath;


void initPath() {
    currentPath = fs::current_path();
    currentPath += "\\Resources\\backgrounds";
}

void openFolderInExplorer() {
    const char* path = currentPath.string().c_str();

    ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
}

bool generateBackgroundPaths() {
    if (!firstPathInit) return true;
    else firstPathInit = false;

    if (!fs::is_directory(currentPath)) {
        if (!fs::create_directory(currentPath)) return false;
    }

    for (const auto& filename : fs::directory_iterator(currentPath)) {
        backgroundPaths.push_back(filename.path().generic_string());
    }
    
    return true;
}

const char* getRandomBackground() {
    static std::string previousBackground = "";
    unsigned int bgPathsSize = backgroundPaths.size();

    while (true) {
        unsigned int randomIndex = gen() % bgPathsSize;

        if (backgroundPaths[randomIndex] != previousBackground or bgPathsSize == 1) {
            previousBackground = backgroundPaths[randomIndex];
            return backgroundPaths[randomIndex].c_str();
        }
    }
}

CCSprite* createBackground(const char* backgroundPath) {
    currentBackgroundPath = backgroundPath;

    auto bg = CCSprite::create(backgroundPath); 
    auto winSize = CCDirector::get()->getWinSize();

    float proportion[2] = {
        bg->getContentWidth() / winSize.width,
        bg->getContentHeight() / winSize.height
    };
    bg->setScaleX(1.0 / proportion[0]);
    bg->setScaleY(1.0 / proportion[1]);

    bg->setPosition(winSize / 2);

    return bg;
}

void changeImage(const char* backgroundPath, CCSprite* bg) {
    currentBackgroundPath = backgroundPath;
    bg->initWithFile(backgroundPath);

    auto winSize = CCDirector::get()->getWinSize();

    float proportion[2] = {
        bg->getContentWidth() / winSize.width,
        bg->getContentHeight() / winSize.height
    };
    bg->setScaleX(1.0 / proportion[0]);
    bg->setScaleY(1.0 / proportion[1]);

    bg->setPosition(winSize / 2);
}

class $modify(CustomMenuLayer, MenuLayer) {

bool init() {
    initPath();

    if (!generateBackgroundPaths()) {
        return MenuLayer::init();
    }

    unsigned int bgPathsSize = backgroundPaths.size();

    if (bgPathsSize == 0) {
        return MenuLayer::init();
    }

    bg = createBackground(getRandomBackground());
    nextBg = CCSprite::create();

    this->addChild(bg);
    this->addChild(nextBg);

    if (!MenuLayer::init()) {
        return false;
    }

    this->getChildren()->removeObjectAtIndex(2);

    this->schedule(
        schedule_selector(CustomMenuLayer::changeBackgroundWithOpacity),
        8.f, kCCRepeatForever, -4.f
    );

    /* Buttons */
    restartBtnSpr = CCSprite::createWithSpriteFrameName(
        "GJ_replayBtn_001.png"
    );
    openFolderBtnSpr = CircleButtonSprite::createWithSpriteFrameName(
        "gj_folderBtn_001.png"
    );

    if (!restartBtnSpr or !openFolderBtnSpr) return false;

    restartBtn = CCMenuItemSpriteExtra::create(
        restartBtnSpr, this, 
        menu_selector(CustomMenuLayer::onButton)
    );

    openFolderBtn = CCMenuItemSpriteExtra::create(
        openFolderBtnSpr, this, 
        menu_selector(CustomMenuLayer::onButton)
    );

    if (!restartBtn or !openFolderBtn) return false;

    restartBtn->setTag(1);
    openFolderBtn->setTag(2);

    auto sideMenu = this->getChildByID("side-menu");
    sideMenu->addChild(restartBtn);
    sideMenu->addChild(openFolderBtn);
    sideMenu->updateLayout();

    return true;
}

void changeBackgroundWithOpacity(float) {
    changeImage(currentBackgroundPath, nextBg);
    changeImage(getRandomBackground(), bg);

    nextBg->setOpacity(255);
    nextBg->runAction(CCFadeOut::create(4));
}

public:

void onButton(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    switch (btn->getTag()) {
        case 1:
            firstPathInit = true;
            backgroundPaths.clear();
            generateBackgroundPaths();
            break;
        case 2:
            openFolderInExplorer();
            break;
        default:
            return;
    }
}

};
