#include "porchlight_guide.h"

#include "core/object/class_db.h"

void PorchlightGuide::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("get_engine_name"),
            &PorchlightGuide::get_engine_name);

    ClassDB::bind_method(
            D_METHOD("get_welcome_message"),
            &PorchlightGuide::get_welcome_message);

    ClassDB::bind_method(
            D_METHOD("celebrate_progress", "milestone"),
            &PorchlightGuide::celebrate_progress);
}

String PorchlightGuide::get_engine_name() const {
    return "Porchlight Engine";
}

String PorchlightGuide::get_welcome_message() const {
    return "Take your time. Every step counts.";
}

String PorchlightGuide::celebrate_progress(
        const String &p_milestone) const {
    const String milestone = p_milestone.strip_edges();

    if (milestone.is_empty()) {
        return "Every step counts.";
    }

    return "You completed: " + milestone + ". Every step counts.";
}
