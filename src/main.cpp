#include <ctime>
#include <string>
#include <vector>
#include <random>
#include <filesystem>

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;
namespace fs = std::filesystem;


std::random_device rd;
std::mt19937 gen(rd());

std::vector<std::string> backgroundPaths;

enum class bgStatus {OK = 1, Transition};
bgStatus status = bgStatus::OK;

CCSprite *bg, *next_bg;

float bgOpacity = 0.f;

const char* current_background_path;


bool generateBackgroundPaths() {
    static bool first_init = true;
    if (!first_init) { return true; }
    else             { first_init = false; }

    auto current_path = fs::current_path();
    current_path += "\\Resources\\backgrounds";

    if (!std::filesystem::is_directory(current_path)) {
        return false;
    }

    for (const auto& filename : fs::directory_iterator(current_path)) {
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
    current_background_path = backgroundPath;

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
    current_background_path = backgroundPath;
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
        if (!generateBackgroundPaths()) {
            return MenuLayer::init();
        }

        unsigned int bgPathsSize = backgroundPaths.size();

        if (bgPathsSize == 0) {
            return MenuLayer::init();
        }

        bg = createBackground(getRandomBackground());
        next_bg = CCSprite::create();

        this->addChild(bg);
        this->addChild(next_bg);

        if (!MenuLayer::init()) {
            return false;
        }

        this->getChildren()->removeObjectAtIndex(2);

        if (bgPathsSize == 1) {
            return true;
        }

        this->schedule(
            schedule_selector(CustomMenuLayer::changeBackgroundWithOpacity),
            8.f, kCCRepeatForever, -4.f
        );

        return true;
    }

    void changeBackgroundWithOpacity(float) {
        changeImage(current_background_path, next_bg);
        changeImage(getRandomBackground(), bg);

        next_bg->setOpacity(255);
        next_bg->runAction(CCFadeOut::create(4));
    }

};
