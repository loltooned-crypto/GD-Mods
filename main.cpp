src/main.cpp
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// -----------------------------------------------------------------------
// A single rectangular indicator bar. Fades/lights up on press, fades
// back out on release, with a tiny scale "pop" so it feels responsive
// rather than just a flat color swap.
// -----------------------------------------------------------------------
class IndicatorBar : public CCNode {
protected:
    CCLayerColor* m_bg  = nullptr;
    CCLayerColor* m_fill = nullptr;
    bool m_active = false;

    bool init(ccColor3B color) {
        if (!CCNode::init()) return false;

        auto barScale = Mod::get()->getSettingValue<double>("bar-scale");
        CCSize size = { 200.f * static_cast<float>(barScale), 22.f * static_cast<float>(barScale) };
        this->setContentSize(size);
        this->setAnchorPoint({ 0.5f, 0.5f });

        m_bg = CCLayerColor::create({ 15, 15, 20, 160 }, size.width, size.height);
        m_bg->setPosition({ -size.width / 2.f, -size.height / 2.f });
        this->addChild(m_bg, 0);

        m_fill = CCLayerColor::create({ color.r, color.g, color.b, 0 }, size.width, size.height);
        m_fill->setPosition({ -size.width / 2.f, -size.height / 2.f });
        this->addChild(m_fill, 1);

        return true;
    }

public:
    static IndicatorBar* create(ccColor3B color) {
        auto ret = new IndicatorBar();
        if (ret->init(color)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void setActive(bool active) {
        if (m_active == active) return;
        m_active = active;

        m_fill->stopAllActions();
        this->stopAllActions();

        if (active) {
            m_fill->runAction(CCFadeTo::create(0.04f, 235));
            this->setScale(1.f);
            this->runAction(CCSequence::create(
                CCScaleTo::create(0.04f, 1.08f),
                CCScaleTo::create(0.08f, 1.f),
                nullptr
            ));
        } else {
            m_fill->runAction(CCFadeTo::create(0.12f, 0));
        }
    }
};

// -----------------------------------------------------------------------
// Holds both players' bars for the current PlayLayer, and plays the
// click/release sounds. Player 2's bar stays hidden until it actually
// sees an event (i.e. only becomes visible once you hit a dual-mode
// section), rather than depending on the level's dual-mode flag.
// -----------------------------------------------------------------------
class ClickIndicatorOverlay : public CCNode {
protected:
    IndicatorBar* m_p1Bar = nullptr;
    IndicatorBar* m_p2Bar = nullptr;
    bool m_p2Seen = false;

    bool init() {
        if (!CCNode::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto p1Color = Mod::get()->getSettingValue<ccColor3B>("player1-color");
        auto p2Color = Mod::get()->getSettingValue<ccColor3B>("player2-color");

        m_p1Bar = IndicatorBar::create(p1Color);
        m_p1Bar->setPosition({ winSize.width / 2.f - 120.f, 44.f });
        this->addChild(m_p1Bar);

        m_p2Bar = IndicatorBar::create(p2Color);
        m_p2Bar->setPosition({ winSize.width / 2.f + 120.f, 44.f });
        m_p2Bar->setOpacity(0);
        this->addChild(m_p2Bar);

        this->setID("click-indicator-overlay"_spr);
        return true;
    }

public:
    static ClickIndicatorOverlay* create() {
        auto ret = new ClickIndicatorOverlay();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void playSound(bool press) {
        if (!Mod::get()->getSettingValue<bool>("enable-sound")) return;
        auto volume = static_cast<float>(Mod::get()->getSettingValue<double>("sound-volume"));
        auto file = press ? "click.ogg"_spr : "release.ogg"_spr;
        // NOTE: verify this against FMODAudioEngine's current signature in your
        // installed bindings (order has historically been file, pitch, pan, volume).
        FMODAudioEngine::sharedEngine()->playEffect(file, 1.f, 0.f, volume);
    }

    void onPress(bool isPlayer2) {
        playSound(true);
        if (isPlayer2) {
            if (!m_p2Seen) {
                m_p2Seen = true;
                m_p2Bar->runAction(CCFadeTo::create(0.15f, 255));
            }
            m_p2Bar->setActive(true);
        } else {
            m_p1Bar->setActive(true);
        }
    }

    void onRelease(bool isPlayer2) {
        playSound(false);
        if (isPlayer2) m_p2Bar->setActive(false);
        else m_p1Bar->setActive(false);
    }
};

// -----------------------------------------------------------------------
// Add the overlay whenever a level is entered.
// -----------------------------------------------------------------------
class $modify(CIPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto overlay = ClickIndicatorOverlay::create();
        this->addChild(overlay, 1000);

        return true;
    }
};

// -----------------------------------------------------------------------
// Detect press/release per player. This fires from game logic itself
// (not raw touch/keyboard events), so it works the same whether you're
// on keyboard, touch, or controller, and it works on any level since it
// never looks at level geometry.
// -----------------------------------------------------------------------
class $modify(CIPlayerObject, PlayerObject) {
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);

        auto pl = PlayLayer::get();
        if (!pl) return;

        auto overlay = typeinfo_cast<ClickIndicatorOverlay*>(pl->getChildByID("click-indicator-overlay"_spr));
        if (!overlay) return;

        overlay->onPress(this == pl->m_player2);
    }

    void releaseButton(PlayerButton btn) {
        PlayerObject::releaseButton(btn);

        auto pl = PlayLayer::get();
        if (!pl) return;

        auto overlay = typeinfo_cast<ClickIndicatorOverlay*>(pl->getChildByID("click-indicator-overlay"_spr));
        if (!overlay) return;

        overlay->onRelease(this == pl->m_player2);
    }
};
