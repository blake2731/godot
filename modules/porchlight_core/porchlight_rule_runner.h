#pragma once

#include "scene/main/node.h"

class PorchlightRule;

class PorchlightRuleRunner : public Node {
    GDCLASS(PorchlightRuleRunner, Node);

    Ref<PorchlightRule> rule;

    bool run_on_ready = true;
    bool run_once = true;
    bool has_run = false;

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    void set_rule(const Ref<PorchlightRule> &p_rule);
    Ref<PorchlightRule> get_rule() const;

    void set_run_on_ready(bool p_run_on_ready);
    bool is_run_on_ready() const;

    void set_run_once(bool p_run_once);
    bool is_run_once() const;

    bool has_run_rule() const;

    bool evaluate_rule();
    void reset_runner();

    String get_description() const;
};
