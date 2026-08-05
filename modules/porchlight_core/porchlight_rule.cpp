#include "porchlight_rule.h"

#include "core/error/error_macros.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

#include "porchlight_condition.h"

void PorchlightRule::_connect_condition() {
    if (condition.is_null()) {
        return;
    }

    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRule::
                            _on_nested_resource_changed);

    if (!condition->is_connected(
                "changed",
                callback)) {
        condition->connect(
                "changed",
                callback);
    }
}

void PorchlightRule::_disconnect_condition() {
    if (condition.is_null()) {
        return;
    }

    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRule::
                            _on_nested_resource_changed);

    if (condition->is_connected(
                "changed",
                callback)) {
        condition->disconnect(
                "changed",
                callback);
    }
}

void PorchlightRule::_connect_actions() {
    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRule::
                            _on_nested_resource_changed);

    if (action_mode == ACTION_SINGLE) {
        if (action.is_null()) {
            return;
        }

        if (!action->is_connected(
                    "changed",
                    callback)) {
            action->connect(
                    "changed",
                    callback);
        }

        return;
    }

    for (int index = 0;
            index < actions.size();
            index++) {
        const Ref<PorchlightAction> current_action =
                actions[index];

        if (current_action.is_null()) {
            continue;
        }

        if (!current_action->is_connected(
                    "changed",
                    callback)) {
            current_action->connect(
                    "changed",
                    callback);
        }
    }
}

void PorchlightRule::_disconnect_actions() {
    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRule::
                            _on_nested_resource_changed);

    if (action_mode == ACTION_SINGLE) {
        if (action.is_null()) {
            return;
        }

        if (action->is_connected(
                    "changed",
                    callback)) {
            action->disconnect(
                    "changed",
                    callback);
        }

        return;
    }

    for (int index = 0;
            index < actions.size();
            index++) {
        const Ref<PorchlightAction> current_action =
                actions[index];

        if (current_action.is_null()) {
            continue;
        }

        if (current_action->is_connected(
                    "changed",
                    callback)) {
            current_action->disconnect(
                    "changed",
                    callback);
        }
    }
}

void PorchlightRule::_on_nested_resource_changed() {
    emit_changed();
}

void PorchlightRule::_validate_property(
        PropertyInfo &p_property) const {
    if (p_property.name == "action" &&
            action_mode != ACTION_SINGLE) {
        p_property.usage &=
                ~PROPERTY_USAGE_EDITOR;
    }

    if (p_property.name == "actions" &&
            action_mode != ACTION_SEQUENCE) {
        p_property.usage &=
                ~PROPERTY_USAGE_EDITOR;
    }
}

void PorchlightRule::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD(
                    "set_condition",
                    "condition"),
            &PorchlightRule::set_condition);

    ClassDB::bind_method(
            D_METHOD("get_condition"),
            &PorchlightRule::get_condition);

    ClassDB::bind_method(
            D_METHOD(
                    "set_action",
                    "action"),
            &PorchlightRule::set_action);

    ClassDB::bind_method(
            D_METHOD("get_action"),
            &PorchlightRule::get_action);

    ClassDB::bind_method(
            D_METHOD(
                    "set_actions",
                    "actions"),
            &PorchlightRule::set_actions);

    ClassDB::bind_method(
            D_METHOD("get_actions"),
            &PorchlightRule::get_actions);

    ClassDB::bind_method(
            D_METHOD(
                    "set_action_mode",
                    "action_mode"),
            &PorchlightRule::set_action_mode);

    ClassDB::bind_method(
            D_METHOD("get_action_mode"),
            &PorchlightRule::get_action_mode);

    ClassDB::bind_method(
            D_METHOD(
                    "set_enabled",
                    "enabled"),
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
            &PorchlightRule::
                    evaluate_and_execute);

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
                    Variant::INT,
                    "action_mode",
                    PROPERTY_HINT_ENUM,
                    "Single,Sequence"),
            "set_action_mode",
            "get_action_mode");

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
                    Variant::ARRAY,
                    "actions",
                    PROPERTY_HINT_ARRAY_TYPE,
                    MAKE_RESOURCE_TYPE_HINT(
                            "PorchlightAction")),
            "set_actions",
            "get_actions");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::BOOL,
                    "enabled"),
            "set_enabled",
            "is_enabled");

    BIND_ENUM_CONSTANT(ACTION_SINGLE);
    BIND_ENUM_CONSTANT(ACTION_SEQUENCE);

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

    _disconnect_condition();

    condition = p_condition;

    _connect_condition();
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

    if (action_mode == ACTION_SINGLE) {
        _disconnect_actions();
    }

    action = p_action;

    if (action_mode == ACTION_SINGLE) {
        _connect_actions();
    }

    emit_changed();
}

Ref<PorchlightAction>
PorchlightRule::get_action() const {
    return action;
}

void PorchlightRule::set_actions(
        const TypedArray<PorchlightAction> &p_actions) {
    if (actions == p_actions) {
        return;
    }

    if (action_mode == ACTION_SEQUENCE) {
        _disconnect_actions();
    }

    actions = p_actions;

    if (action_mode == ACTION_SEQUENCE) {
        _connect_actions();
    }

    emit_changed();
}

TypedArray<PorchlightAction>
PorchlightRule::get_actions() const {
    return actions;
}

void PorchlightRule::set_action_mode(
        ActionMode p_action_mode) {
    ERR_FAIL_COND(
            p_action_mode < ACTION_SINGLE ||
            p_action_mode > ACTION_SEQUENCE);

    if (action_mode == p_action_mode) {
        return;
    }

    _disconnect_actions();

    action_mode = p_action_mode;

    _connect_actions();

    notify_property_list_changed();
    emit_changed();
}

PorchlightRule::ActionMode
PorchlightRule::get_action_mode() const {
    return action_mode;
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
    if (condition.is_null() ||
            !condition->is_valid()) {
        return false;
    }

    if (action_mode == ACTION_SINGLE) {
        return action.is_valid() &&
                action->is_valid();
    }

    if (actions.is_empty()) {
        return false;
    }

    for (int index = 0;
            index < actions.size();
            index++) {
        const Ref<PorchlightAction> current_action =
                actions[index];

        if (current_action.is_null() ||
                !current_action->is_valid()) {
            return false;
        }
    }

    return true;
}

bool PorchlightRule::is_condition_met() const {
    if (!enabled || condition.is_null()) {
        return false;
    }

    return condition->is_met();
}

bool PorchlightRule::evaluate_and_execute() {
    if (!enabled || !is_valid()) {
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
        if (action_mode == ACTION_SINGLE) {
            action_changed =
                    action->execute();
        } else {
            for (int index = 0;
                    index < actions.size();
                    index++) {
                const Ref<PorchlightAction>
                        current_action =
                                actions[index];

                const bool current_changed =
                        current_action->execute();

                if (current_changed) {
                    action_changed = true;
                }
            }
        }
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

    if (condition.is_null()) {
        if (action_mode == ACTION_SINGLE &&
                action.is_null()) {
            return "Rule has no condition or action.";
        }

        if (action_mode == ACTION_SEQUENCE &&
                actions.is_empty()) {
            return "Rule has no condition or actions.";
        }

        return "Rule has no condition.";
    }

    if (action_mode == ACTION_SINGLE) {
        if (action.is_null()) {
            return "Rule has no action.";
        }

        return String("IF ") +
                condition->get_description() +
                " THEN " +
                action->get_description();
    }

    if (actions.is_empty()) {
        return "Rule has no actions.";
    }

    PackedStringArray action_descriptions;

    for (int index = 0;
            index < actions.size();
            index++) {
        const Ref<PorchlightAction> current_action =
                actions[index];

        if (current_action.is_null()) {
            action_descriptions.push_back(
                    "No action assigned.");

            continue;
        }

        action_descriptions.push_back(
                current_action->get_description());
    }

    return String("IF ") +
            condition->get_description() +
            " THEN " +
            String(" THEN ").join(
                    action_descriptions);
}
