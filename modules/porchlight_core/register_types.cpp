#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"

#include "porchlight_guide.h"
#include "porchlight_progress.h"

static PorchlightProgress *porchlight_progress = nullptr;

void initialize_porchlight_core_module(
        ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    GDREGISTER_CLASS(PorchlightGuide);
    GDREGISTER_CLASS(PorchlightProgress);

    porchlight_progress = memnew(PorchlightProgress);

    Engine::get_singleton()->add_singleton(
            Engine::Singleton(
                    "PorchlightProgress",
                    porchlight_progress));
}

void uninitialize_porchlight_core_module(
        ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (porchlight_progress != nullptr) {
        Engine::get_singleton()->remove_singleton(
                "PorchlightProgress");

        memdelete(porchlight_progress);
        porchlight_progress = nullptr;
    }
}
