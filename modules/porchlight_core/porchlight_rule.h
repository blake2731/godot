#pragma once

#include "core/io/resource.h"

class PorchlightAction;
class PorchlightCondition;

class PorchlightRule : public Resource {
    GDCLASS(PorchlightRule, Resource);

    Ref<PorchlightCondition> condition;
    Ref<PorchlightAction> action;

    bool enabled = true;

protected:
    static void _bind_methods();

public:
    void set_condition(
            const Ref<PorchlightCondition> &p_condition);
    Ref<PorchlightCondition> get_condition() const;

    void set_action(
            const Ref<PorchlightAction> &p_action);
    Ref<PorchlightAction> get_action() const;

    void set_enabled(bool p_enabled);
    bool is_enabled() const;

    bool is_valid() const;
    bool is_condition_met() const;
    bool evaluate_and_execute();

    String get_description() const;
};
