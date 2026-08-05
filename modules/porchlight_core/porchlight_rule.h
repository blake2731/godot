#pragma once

#include "core/io/resource.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"

#include "porchlight_action.h"

class PorchlightCondition;

class PorchlightRule : public Resource {
    GDCLASS(PorchlightRule, Resource);

public:
    enum ActionMode {
        ACTION_SINGLE = 0,
        ACTION_SEQUENCE = 1,
    };

private:
    Ref<PorchlightCondition> condition;

    Ref<PorchlightAction> action;
    TypedArray<PorchlightAction> actions;

    ActionMode action_mode = ACTION_SINGLE;
    bool enabled = true;

    void _connect_condition();
    void _disconnect_condition();

    void _connect_actions();
    void _disconnect_actions();

    void _on_nested_resource_changed();

protected:
    static void _bind_methods();

    void _validate_property(
            PropertyInfo &p_property) const;

public:
    void set_condition(
            const Ref<PorchlightCondition> &p_condition);

    Ref<PorchlightCondition> get_condition() const;

    void set_action(
            const Ref<PorchlightAction> &p_action);

    Ref<PorchlightAction> get_action() const;

    void set_actions(
            const TypedArray<PorchlightAction> &p_actions);

    TypedArray<PorchlightAction> get_actions() const;

    void set_action_mode(ActionMode p_action_mode);
    ActionMode get_action_mode() const;

    void set_enabled(bool p_enabled);
    bool is_enabled() const;

    bool is_valid() const;
    bool is_condition_met() const;
    bool evaluate_and_execute();

    String get_description() const;
};

VARIANT_ENUM_CAST(PorchlightRule::ActionMode);
