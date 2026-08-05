#include "porchlight_condition.h"

#include "core/config/engine.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"

#include "porchlight_progress.h"

PackedStringArray
PorchlightCondition::_get_normalized_milestones() const {
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

void PorchlightCondition::_validate_property(
        PropertyInfo &p_property) const {
    if (p_property.name == "milestone" &&
            match_mode != MATCH_SINGLE) {
        p_property.usage &=
                ~PROPERTY_USAGE_EDITOR;
    }

    if (p_property.name == "milestones" &&
            match_mode == MATCH_SINGLE) {
        p_property.usage &=
                ~PROPERTY_USAGE_EDITOR;
    }
}

void PorchlightCondition::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("set_milestone", "milestone"),
            &PorchlightCondition::set_milestone);

    ClassDB::bind_method(
            D_METHOD("get_milestone"),
            &PorchlightCondition::get_milestone);

    ClassDB::bind_method(
            D_METHOD("set_milestones", "milestones"),
            &PorchlightCondition::set_milestones);

    ClassDB::bind_method(
            D_METHOD("get_milestones"),
            &PorchlightCondition::get_milestones);

    ClassDB::bind_method(
            D_METHOD("set_match_mode", "match_mode"),
            &PorchlightCondition::set_match_mode);

    ClassDB::bind_method(
            D_METHOD("get_match_mode"),
            &PorchlightCondition::get_match_mode);

    ClassDB::bind_method(
            D_METHOD("set_inverted", "inverted"),
            &PorchlightCondition::set_inverted);

    ClassDB::bind_method(
            D_METHOD("is_inverted"),
            &PorchlightCondition::is_inverted);

    ClassDB::bind_method(
            D_METHOD("is_valid"),
            &PorchlightCondition::is_valid);

    ClassDB::bind_method(
            D_METHOD(
                    "references_milestone",
                    "milestone"),
            &PorchlightCondition::
                    references_milestone);

    ClassDB::bind_method(
            D_METHOD("is_met"),
            &PorchlightCondition::is_met);

    ClassDB::bind_method(
            D_METHOD("get_description"),
            &PorchlightCondition::get_description);

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::INT,
                    "match_mode",
                    PROPERTY_HINT_ENUM,
                    "Single,All,Any"),
            "set_match_mode",
            "get_match_mode");

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
                    Variant::BOOL,
                    "inverted"),
            "set_inverted",
            "is_inverted");

    BIND_ENUM_CONSTANT(MATCH_SINGLE);
    BIND_ENUM_CONSTANT(MATCH_ALL);
    BIND_ENUM_CONSTANT(MATCH_ANY);
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

void PorchlightCondition::set_milestones(
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
PorchlightCondition::get_milestones() const {
    return milestones;
}

void PorchlightCondition::set_match_mode(
        MatchMode p_match_mode) {
    ERR_FAIL_COND(
            p_match_mode < MATCH_SINGLE ||
            p_match_mode > MATCH_ANY);

    if (match_mode == p_match_mode) {
        return;
    }

    match_mode = p_match_mode;

    notify_property_list_changed();
    emit_changed();
}

PorchlightCondition::MatchMode
PorchlightCondition::get_match_mode() const {
    return match_mode;
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

bool PorchlightCondition::is_valid() const {
    if (match_mode == MATCH_SINGLE) {
        return !String(milestone)
                        .strip_edges()
                        .is_empty();
    }

    return !_get_normalized_milestones()
                    .is_empty();
}

bool PorchlightCondition::references_milestone(
        const StringName &p_milestone) const {
    const String normalized_text =
            String(p_milestone).strip_edges();

    if (normalized_text.is_empty()) {
        return false;
    }

    if (match_mode == MATCH_SINGLE) {
        return String(milestone) ==
                normalized_text;
    }

    return _get_normalized_milestones()
            .has(normalized_text);
}

bool PorchlightCondition::is_met() const {
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

    bool condition_met = false;

    if (match_mode == MATCH_SINGLE) {
        condition_met =
                progress->is_complete(
                        milestone);
    } else {
        const PackedStringArray
                normalized_milestones =
                        _get_normalized_milestones();

        if (match_mode == MATCH_ALL) {
            condition_met = true;

            for (int index = 0;
                    index <
                    normalized_milestones.size();
                    index++) {
                if (!progress->is_complete(
                            StringName(
                                    normalized_milestones[
                                            index]))) {
                    condition_met = false;
                    break;
                }
            }
        } else {
            condition_met = false;

            for (int index = 0;
                    index <
                    normalized_milestones.size();
                    index++) {
                if (progress->is_complete(
                            StringName(
                                    normalized_milestones[
                                            index]))) {
                    condition_met = true;
                    break;
                }
            }
        }
    }

    return inverted
            ? !condition_met
            : condition_met;
}

String PorchlightCondition::
get_description() const {
    if (!is_valid()) {
        if (match_mode == MATCH_SINGLE) {
            return "No milestone selected.";
        }

        return "No milestones selected.";
    }

    if (match_mode == MATCH_SINGLE) {
        const String milestone_text =
                String(milestone);

        if (inverted) {
            return String(
                    "Milestone is not complete: ") +
                    milestone_text;
        }

        return String(
                "Milestone is complete: ") +
                milestone_text;
    }

    const String milestone_list =
            String(", ").join(
                    _get_normalized_milestones());

    if (match_mode == MATCH_ALL) {
        if (inverted) {
            return String(
                    "Not all milestones are complete: ") +
                    milestone_list;
        }

        return String(
                "All milestones are complete: ") +
                milestone_list;
    }

    if (inverted) {
        return String(
                "No milestones are complete: ") +
                milestone_list;
    }

    return String(
            "At least one milestone is complete: ") +
            milestone_list;
}
