#include "porchlight_condition.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"

#include "porchlight_progress.h"

void PorchlightCondition::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("set_milestone", "milestone"),
            &PorchlightCondition::set_milestone);

    ClassDB::bind_method(
            D_METHOD("get_milestone"),
            &PorchlightCondition::get_milestone);

    ClassDB::bind_method(
            D_METHOD("set_inverted", "inverted"),
            &PorchlightCondition::set_inverted);

    ClassDB::bind_method(
            D_METHOD("is_inverted"),
            &PorchlightCondition::is_inverted);

    ClassDB::bind_method(
            D_METHOD("is_met"),
            &PorchlightCondition::is_met);

    ClassDB::bind_method(
            D_METHOD("get_description"),
            &PorchlightCondition::get_description);

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::STRING_NAME,
                    "milestone"),
            "set_milestone",
            "get_milestone");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::BOOL,
                    "inverted"),
            "set_inverted",
            "is_inverted");
}

void PorchlightCondition::set_milestone(
        const StringName &p_milestone) {
    const String normalized_text =
            String(p_milestone).strip_edges();

    const StringName normalized_milestone =
            normalized_text;

    if (milestone == normalized_milestone) {
        return;
    }

    milestone = normalized_milestone;
    emit_changed();
}

StringName PorchlightCondition::get_milestone() const {
    return milestone;
}

void PorchlightCondition::set_inverted(
        bool p_inverted) {
    if (inverted == p_inverted) {
        return;
    }

    inverted = p_inverted;
    emit_changed();
}

bool PorchlightCondition::is_inverted() const {
    return inverted;
}

bool PorchlightCondition::is_met() const {
    const String milestone_text =
            String(milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return false;
    }

    Object *singleton_object =
            Engine::get_singleton()
                    ->get_singleton_object(
                            "PorchlightProgress");

    PorchlightProgress *progress =
            Object::cast_to<PorchlightProgress>(
                    singleton_object);

    if (progress == nullptr) {
        return false;
    }

    const bool milestone_complete =
            progress->is_complete(
                    StringName(milestone_text));

    return inverted
            ? !milestone_complete
            : milestone_complete;
}

String PorchlightCondition::get_description() const {
    const String milestone_text =
            String(milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return "No milestone selected.";
    }

    if (inverted) {
        return String(
                "Milestone is not complete: ") +
                milestone_text;
    }

    return String(
            "Milestone is complete: ") +
            milestone_text;
}
