#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include <vector>
#include <random>
#include <string>
#include <filesystem>

using namespace geode;
using namespace geode::prelude;

namespace fs = std::filesystem;

std::random_device rd;
std::mt19937 gen(rd());

class backgroundNode : public CCNode {
private:

CCSprite* m_currentBg;
float m_fadeOutTime = 2.f;
float m_bgChangeTime = 2.f;

public:

virtual bool init(std::vector<std::string>* bgPaths) {
    if (!CCNode::init()) return false;
    if (bgPaths == nullptr or bgPaths->size() == 0) return false;

    auto winSize = CCDirector::get()->getWinSize();

    for (auto& path : *bgPaths) {
        auto sprite = CCSprite::create(path.c_str());

        if (!sprite) continue;

        float proportion[2] = {
            sprite->getContentWidth() / winSize.width,
            sprite->getContentHeight() / winSize.height
        };
        sprite->setScaleX(1.0 / proportion[0]);
        sprite->setScaleY(1.0 / proportion[1]);

        sprite->setPosition(winSize / 2);

        sprite->setOpacity(0);

        this->addChild(sprite);
    }

    auto currentBg = reinterpret_cast<CCSprite*>(
        this->getChildren()->objectAtIndex(gen() % this->getChildrenCount())
    );
    currentBg->setOpacity(255);
    m_currentBg = currentBg;

    return true;
}

void setRandomBg() {
    auto objArray = this->getChildren();
    CCSprite* randomBgSprite;

    if (objArray->count() <= 1) return;

    while (true) {
        unsigned int randomBg = gen() % this->getChildrenCount();

        randomBgSprite = reinterpret_cast<CCSprite*>(
            objArray->objectAtIndex(randomBg)
        );

        if (randomBgSprite != m_currentBg) break;
    }

    unsigned int indexCurrentBg = objArray->indexOfObject(m_currentBg);
    unsigned int arraySize = this->getChildrenCount();
    objArray->exchangeObjectAtIndex(indexCurrentBg, arraySize - 1);

    randomBgSprite->setOpacity(255);
    m_currentBg->runAction(CCFadeOut::create(m_fadeOutTime));

    m_currentBg = randomBgSprite;
}

const float& getFadeOutTime() const {
    return m_fadeOutTime;
}

const float& getBgChangeTime() const {
    return m_bgChangeTime;
}

static backgroundNode* create(std::vector<std::string>* bgPaths) {
    auto ret = new backgroundNode;

    if (ret->init(bgPaths)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

static std::vector<std::string>* generateBgPaths() {
    auto bgPaths = new std::vector<std::string>;

    fs::path backgroundPath = fs::current_path();
    backgroundPath += "\\Resources\\backgrounds";

    if (!fs::is_directory(backgroundPath)) {
        if (!fs::create_directory(backgroundPath)) return nullptr;
    }

    for (const auto& filename : fs::directory_iterator(backgroundPath)) {
        bgPaths->push_back(filename.path().generic_string());
    }

    return bgPaths;
}

};


class $modify(CustomMenuLayer, MenuLayer) {

struct Fields {
    backgroundNode* bg;
};

void setRandomBg(float) {
    m_fields->bg->setRandomBg();
}

bool init() {
    auto bgPaths = backgroundNode::generateBgPaths();
    auto bg = backgroundNode::create(bgPaths);

    if (bg == nullptr) return MenuLayer::init();

    this->addChild(bg);
    m_fields->bg = bg;

    if (!MenuLayer::init()) return false;

    this->getChildren()->removeObjectAtIndex(1);

    float fadeOutTime = bg->getFadeOutTime();
    float bgChangeTime = bg->getBgChangeTime();

    this->schedule(
        schedule_selector(CustomMenuLayer::setRandomBg),
        bgChangeTime + fadeOutTime, kCCRepeatForever, 0.f
    );

    return true;
}

};
