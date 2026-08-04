#include "porchlight_action.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"

#include "porchlight_progress.h"

void PorchlightAction::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("set_milestone", "milestone"),
            &PorchlightAction::set_milestone);

    ClassDB::bind_method(
            D_METHOD("get_milestone"),
            &PorchlightAction::get_milestone);

    ClassDB::bind_method(
            D_METHOD("is_valid"),
            &PorchlightAction::is_valid);

    ClassDB::bind_method(
            D_METHOD("execute"),
            &PorchlightAction::execute);

    ClassDB::bind_method(
            D_METHOD("get_description"),
            &PorchlightAction::get_description);

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::STRING_NAME,
                    "milestone"),
            "set_milestone",
            "get_milestone");

    ADD_SIGNAL(
            MethodInfo(
                    "executed",
                    PropertyInfo(
                            Variant::STRING_NAME,
                            "milestone"),
                    PropertyInfo(
                            Variant::BOOL,
                            "changed")));
}

void PorchlightAction::set_milestone(
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

StringName PorchlightAction::get_milestone() const {
    return milestone;
}

bool PorchlightAction::is_valid() const {
    return !String(milestone).strip_edges().is_empty();
}

bool PorchlightAction::execute() {
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

    const bool changed =
            progress->complete_milestone(
                    StringName(milestone_text));

    emit_signal(
            "executed",
            StringName(milestone_text),
            changed);

    return changed;
}

String PorchlightAction::get_description() const {
    const String milestone_text =
            String(milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return "No milestone selected.";
    }

    return String("Complete milestone: ") +
            milestone_text;
}
