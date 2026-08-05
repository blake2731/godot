#pragma once

#include "scene/main/node.h"

class PorchlightProgress;
class PorchlightRule;

class PorchlightRuleRunner : public Node {
    GDCLASS(PorchlightRuleRunner, Node);

    Ref<PorchlightRule> rule;

    bool run_on_ready = true;
    bool watch_progress = true;
    bool run_once = true;
    bool has_run = false;
    bool is_evaluating = false;

    PorchlightProgress *_get_progress() const;

    void _connect_rule();
    void _disconnect_rule();

    void _connect_progress();
    void _disconnect_progress();

    void _evaluate_for_milestone(
            const StringName &p_milestone);

    void _on_rule_changed();

    void _on_milestone_completed(
            const StringName &p_milestone);

    void _on_milestone_removed(
            const StringName &p_milestone);

    void _on_milestones_cleared();

    void _on_progress_reloaded(
            const PackedStringArray &p_added_milestones,
            const PackedStringArray &p_removed_milestones);

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    void set_rule(const Ref<PorchlightRule> &p_rule);
    Ref<PorchlightRule> get_rule() const;

    void set_run_on_ready(bool p_run_on_ready);
    bool is_run_on_ready() const;

    void set_watch_progress(bool p_watch_progress);
    bool is_watch_progress() const;

    void set_run_once(bool p_run_once);
    bool is_run_once() const;

    bool has_run_rule() const;

    bool evaluate_rule();
    void reset_runner();

    String get_description() const;

    PackedStringArray get_setup_warnings() const;

    PackedStringArray get_configuration_warnings()
            const override;
};
