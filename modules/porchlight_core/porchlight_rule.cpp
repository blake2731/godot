#include "porchlight_rule.h"

#include "core/object/class_db.h"

#include "porchlight_action.h"
#include "porchlight_condition.h"

void PorchlightRule::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("set_condition", "condition"),
            &PorchlightRule::set_condition);

    ClassDB::bind_method(
            D_METHOD("get_condition"),
            &PorchlightRule::get_condition);

    ClassDB::bind_method(
            D_METHOD("set_action", "action"),
            &PorchlightRule::set_action);

    ClassDB::bind_method(
            D_METHOD("get_action"),
            &PorchlightRule::get_action);

    ClassDB::bind_method(
            D_METHOD("set_enabled", "enabled"),
            &PorchlightRule::set_enabled);

    ClassDB::bind_method(
            D_METHOD("is_enabled"),
            &PorchlightRule::is_enabled);

    ClassDB::bind_method(
            D_METHOD("is_valid"),
            &PorchlightRule::is_valid);

    ClassDB::bind_method(
            D_METHOD("is_condition_met"),
            &PorchlightRule::is_condition_met);

    ClassDB::bind_method(
            D_METHOD("evaluate_and_execute"),
            &PorchlightRule::evaluate_and_execute);

    ClassDB::bind_method(
            D_METHOD("get_description"),
            &PorchlightRule::get_description);

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::OBJECT,
                    "condition",
                    PROPERTY_HINT_RESOURCE_TYPE,
                    "PorchlightCondition"),
            "set_condition",
            "get_condition");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::OBJECT,
                    "action",
                    PROPERTY_HINT_RESOURCE_TYPE,
                    "PorchlightAction"),
            "set_action",
            "get_action");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::BOOL,
                    "enabled"),
            "set_enabled",
            "is_enabled");

    ADD_SIGNAL(
            MethodInfo(
                    "evaluated",
                    PropertyInfo(
                            Variant::BOOL,
                            "condition_met"),
                    PropertyInfo(
                            Variant::BOOL,
                            "action_changed")));
}

void PorchlightRule::set_condition(
        const Ref<PorchlightCondition> &p_condition) {
    if (condition == p_condition) {
        return;
    }

    condition = p_condition;
    emit_changed();
}

Ref<PorchlightCondition>
PorchlightRule::get_condition() const {
    return condition;
}

void PorchlightRule::set_action(
        const Ref<PorchlightAction> &p_action) {
    if (action == p_action) {
        return;
    }

    action = p_action;
    emit_changed();
}

Ref<PorchlightAction>
PorchlightRule::get_action() const {
    return action;
}

void PorchlightRule::set_enabled(bool p_enabled) {
    if (enabled == p_enabled) {
        return;
    }

    enabled = p_enabled;
    emit_changed();
}

bool PorchlightRule::is_enabled() const {
    return enabled;
}

bool PorchlightRule::is_valid() const {
    if (condition.is_null() || action.is_null()) {
        return false;
    }

    const String condition_milestone =
            String(
                    condition->get_milestone())
                    .strip_edges();

    if (condition_milestone.is_empty()) {
        return false;
    }

    return action->is_valid();
}

bool PorchlightRule::is_condition_met() const {
    if (!enabled || condition.is_null()) {
        return false;
    }

    return condition->is_met();
}

bool PorchlightRule::evaluate_and_execute() {
    if (!enabled ||
            condition.is_null() ||
            action.is_null()) {
        emit_signal(
                "evaluated",
                false,
                false);

        return false;
    }

    const bool condition_met =
            condition->is_met();

    bool action_changed = false;

    if (condition_met) {
        action_changed = action->execute();
    }

    emit_signal(
            "evaluated",
            condition_met,
            action_changed);

    return action_changed;
}

String PorchlightRule::get_description() const {
    if (!enabled) {
        return "Rule disabled.";
    }

    if (condition.is_null() && action.is_null()) {
        return "Rule has no condition or action.";
    }

    if (condition.is_null()) {
        return "Rule has no condition.";
    }

    if (action.is_null()) {
        return "Rule has no action.";
    }

    return String("IF ") +
            condition->get_description() +
            " THEN " +
            action->get_description();
}
