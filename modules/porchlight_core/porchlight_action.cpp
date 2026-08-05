#include "porchlight_action.h"

#include "core/config/engine.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"

#include "porchlight_progress.h"

PackedStringArray
PorchlightAction::_get_normalized_milestones() const {
    PackedStringArray normalized_milestones;

    for (int index = 0;
            index < milestones.size();
            index++) {
        const String normalized_text =
                milestones[index].strip_edges();

        if (normalized_text.is_empty()) {
            continue;
        }

        if (normalized_milestones.has(
                    normalized_text)) {
            continue;
        }

        normalized_milestones.push_back(
                normalized_text);
    }

    return normalized_milestones;
}

void PorchlightAction::_validate_property(
        PropertyInfo &p_property) const {
    if (p_property.name == "milestone" &&
            target_mode != TARGET_SINGLE) {
        p_property.usage &=
                ~PROPERTY_USAGE_EDITOR;
    }

    if (p_property.name == "milestones" &&
            target_mode == TARGET_SINGLE) {
        p_property.usage &=
                ~PROPERTY_USAGE_EDITOR;
    }
}

void PorchlightAction::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD(
                    "set_milestone",
                    "milestone"),
            &PorchlightAction::set_milestone);

    ClassDB::bind_method(
            D_METHOD("get_milestone"),
            &PorchlightAction::get_milestone);

    ClassDB::bind_method(
            D_METHOD(
                    "set_milestones",
                    "milestones"),
            &PorchlightAction::set_milestones);

    ClassDB::bind_method(
            D_METHOD("get_milestones"),
            &PorchlightAction::get_milestones);

    ClassDB::bind_method(
            D_METHOD(
                    "set_operation",
                    "operation"),
            &PorchlightAction::set_operation);

    ClassDB::bind_method(
            D_METHOD("get_operation"),
            &PorchlightAction::get_operation);

    ClassDB::bind_method(
            D_METHOD(
                    "set_target_mode",
                    "target_mode"),
            &PorchlightAction::set_target_mode);

    ClassDB::bind_method(
            D_METHOD("get_target_mode"),
            &PorchlightAction::get_target_mode);

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
                    Variant::INT,
                    "target_mode",
                    PROPERTY_HINT_ENUM,
                    "Single,Multiple"),
            "set_target_mode",
            "get_target_mode");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::STRING_NAME,
                    "milestone"),
            "set_milestone",
            "get_milestone");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::PACKED_STRING_ARRAY,
                    "milestones"),
            "set_milestones",
            "get_milestones");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::INT,
                    "operation",
                    PROPERTY_HINT_ENUM,
                    "Complete,Remove"),
            "set_operation",
            "get_operation");

    BIND_ENUM_CONSTANT(OPERATION_COMPLETE);
    BIND_ENUM_CONSTANT(OPERATION_REMOVE);

    BIND_ENUM_CONSTANT(TARGET_SINGLE);
    BIND_ENUM_CONSTANT(TARGET_MULTIPLE);

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

void PorchlightAction::set_milestones(
        const PackedStringArray &p_milestones) {
    PackedStringArray normalized_milestones;

    for (int index = 0;
            index < p_milestones.size();
            index++) {
        const String normalized_text =
                p_milestones[index].strip_edges();

        if (normalized_text.is_empty()) {
            continue;
        }

        if (normalized_milestones.has(
                    normalized_text)) {
            continue;
        }

        normalized_milestones.push_back(
                normalized_text);
    }

    if (milestones == normalized_milestones) {
        return;
    }

    milestones = normalized_milestones;
    emit_changed();
}

PackedStringArray
PorchlightAction::get_milestones() const {
    return milestones;
}

void PorchlightAction::set_operation(
        Operation p_operation) {
    ERR_FAIL_COND(
            p_operation < OPERATION_COMPLETE ||
            p_operation > OPERATION_REMOVE);

    if (operation == p_operation) {
        return;
    }

    operation = p_operation;
    emit_changed();
}

PorchlightAction::Operation
PorchlightAction::get_operation() const {
    return operation;
}

void PorchlightAction::set_target_mode(
        TargetMode p_target_mode) {
    ERR_FAIL_COND(
            p_target_mode < TARGET_SINGLE ||
            p_target_mode > TARGET_MULTIPLE);

    if (target_mode == p_target_mode) {
        return;
    }

    target_mode = p_target_mode;

    notify_property_list_changed();
    emit_changed();
}

PorchlightAction::TargetMode
PorchlightAction::get_target_mode() const {
    return target_mode;
}

bool PorchlightAction::is_valid() const {
    if (target_mode == TARGET_SINGLE) {
        return !String(milestone)
                        .strip_edges()
                        .is_empty();
    }

    return !_get_normalized_milestones()
                    .is_empty();
}

bool PorchlightAction::execute() {
    if (!is_valid()) {
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

    if (target_mode == TARGET_SINGLE) {
        const StringName target_milestone =
                String(milestone).strip_edges();

        bool changed = false;

        switch (operation) {
            case OPERATION_COMPLETE: {
                changed =
                        progress->complete_milestone(
                                target_milestone);
            } break;

            case OPERATION_REMOVE: {
                changed =
                        progress->remove_milestone(
                                target_milestone);
            } break;
        }

        emit_signal(
                "executed",
                target_milestone,
                changed);

        return changed;
    }

    const PackedStringArray target_milestones =
            _get_normalized_milestones();

    PackedStringArray changed_milestones;

    switch (operation) {
        case OPERATION_COMPLETE: {
            changed_milestones =
                    progress->complete_milestones(
                            target_milestones);
        } break;

        case OPERATION_REMOVE: {
            changed_milestones =
                    progress->remove_milestones(
                            target_milestones);
        } break;
    }

    for (int index = 0;
            index < target_milestones.size();
            index++) {
        const StringName target_milestone =
                target_milestones[index];

        const bool changed =
                changed_milestones.has(
                        target_milestones[index]);

        emit_signal(
                "executed",
                target_milestone,
                changed);
    }

    return !changed_milestones.is_empty();
}

String PorchlightAction::get_description() const {
    if (!is_valid()) {
        if (target_mode == TARGET_SINGLE) {
            return "No milestone selected.";
        }

        return "No milestones selected.";
    }

    if (target_mode == TARGET_SINGLE) {
        const String milestone_text =
                String(milestone);

        switch (operation) {
            case OPERATION_COMPLETE:
                return String(
                        "Complete milestone: ") +
                        milestone_text;

            case OPERATION_REMOVE:
                return String(
                        "Remove milestone: ") +
                        milestone_text;
        }
    }

    const String milestone_list =
            String(", ").join(
                    _get_normalized_milestones());

    switch (operation) {
        case OPERATION_COMPLETE:
            return String(
                    "Complete milestones: ") +
                    milestone_list;

        case OPERATION_REMOVE:
            return String(
                    "Remove milestones: ") +
                    milestone_list;
    }

    return "Unknown milestone operation.";
}
