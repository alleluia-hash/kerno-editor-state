// State Readout & Timeline (editor).
// Adds a "State" button to the level editor. Click it to see the vehicle, speed,
// gravity, size and mirror state at the first Start Position, and the timeline
// of changes that lead to it. Logs the timeline to the console too.
//
// UNVERIFIED (check if the build or readout looks wrong):
// - m_objects, m_startPosObjects (LevelEditorLayer)
// - m_level->m_levelSettings and its fields
// - portal object IDs in the tables below

#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/GameObject.hpp>
#include <algorithm>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace kerno_editor {
    struct State {
        int speed = 1;        // index into SPEED_NAMES
        int mode = 0;         // index into MODE_NAMES
        bool inverted = false;
        bool mini = false;
        bool mirror = false;
    };

    static const char* SPEED_NAMES[] = {"Slow", "Normal", "2x", "3x", "4x"};
    static const char* MODE_NAMES[] = {"Cube", "Ship", "Ball", "UFO", "Wave", "Robot", "Spider", "Swing"};

    // Portal IDs (from memory, verify).
    static int speedIndex(int id) {
        switch (id) {
            case 200: return 0;
            case 201: return 1;
            case 202: return 2;
            case 203: return 3;
            case 1334: return 4;
            default: return -1;
        }
    }

    static int modeIndex(int id) {
        switch (id) {
            case 12: return 0;
            case 13: return 1;
            case 47: return 2;
            case 111: return 3;
            case 660: return 4;
            case 745: return 5;
            case 1331: return 6;
            case 1933: return 7;
            default: return -1;
        }
    }

    static std::string describe(State const& s) {
        std::string speed = SPEED_NAMES[std::clamp(s.speed, 0, 4)];
        std::string mode = MODE_NAMES[std::clamp(s.mode, 0, 7)];
        return "[ Vehicle: " + mode +
               " | Speed: " + speed +
               " | Gravity: " + (s.inverted ? "Inverted" : "Normal") +
               " | Size: " + (s.mini ? "Mini" : "Normal") +
               " | Mirror: " + (s.mirror ? "On" : "Off") + " ]";
    }
}

class $modify(KernoEditor, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelEditorLayer::init(level, p1)) return false;

        auto sprite = ButtonSprite::create("State", 90, false, "goldFont.fnt", "GJ_button_01.png", 28.f, 0.6f);
        auto item = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(KernoEditor::onReadout));
        auto menu = CCMenu::create();
        menu->addChild(item);

        // Position is a guess; adjust it if it overlaps editor buttons.
        auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition(size.width * 0.1f, size.height * 0.1f);
        this->addChild(menu);
        return true;
    }

    void onReadout(CCObject*) {
        if (!this->m_startPosObjects || this->m_startPosObjects->count() == 0) {
            Notification::create("No Start Position placed", NotificationIcon::Warning, 2.f)->show();
            return;
        }

        auto startPos = static_cast<GameObject*>(this->m_startPosObjects->objectAtIndex(0));
        float stopX = startPos->getPositionX();

        // Collect every object before the Start Position, sorted left to right.
        std::vector<GameObject*> before;
        if (this->m_objects) {
            for (auto* obj : CCArrayExt<GameObject*>(this->m_objects)) {
                if (obj->getPositionX() < stopX) before.push_back(obj);
            }
        }
        std::sort(before.begin(), before.end(), [](GameObject* a, GameObject* b) {
            return a->getPositionX() < b->getPositionX();
        });

        // Starting state from the level settings.
        kerno_editor::State state;
        auto settings = this->m_level->m_levelSettings;
        if (settings) {
            state.speed = static_cast<int>(settings->m_startSpeed);
            state.mode = settings->m_startMode;
            state.mini = settings->m_startMini;
            state.inverted = settings->m_isFlipped;
            state.mirror = settings->m_mirrorMode;
        }

        log::info("Timeline start: {}", kerno_editor::describe(state));

        // Walk the timeline, applying each portal in order.
        for (auto* obj : before) {
            int id = obj->m_objectID;
            float x = obj->getPositionX();
            bool changed = true;

            if (int s = kerno_editor::speedIndex(id); s >= 0) state.speed = s;
            else if (int m = kerno_editor::modeIndex(id); m >= 0) state.mode = m;
            else if (id == 10) state.inverted = false;   // gravity portal, verify which ID is which
            else if (id == 11) state.inverted = true;
            else if (id == 99) state.mini = false;       // size portal, verify
            else if (id == 101) state.mini = true;
            else if (id == 45) state.mirror = true;      // mirror portal, verify
            else if (id == 46) state.mirror = false;
            else changed = false;

            if (changed) {
                log::info("{} units: {}", static_cast<int>(x), kerno_editor::describe(state));
            }
        }

        std::string text = "Start Pos " + std::to_string(static_cast<int>(stopX)) + ": " + kerno_editor::describe(state);
        Notification::create(text, NotificationIcon::Info, 5.f)->show();
    }
};
