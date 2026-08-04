#include "register_types.h"

#include "core/object/class_db.h"
#include "porchlight_guide.h"

void initialize_porchlight_core_module(
        ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_class<PorchlightGuide>();
}

void uninitialize_porchlight_core_module(
        ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}
