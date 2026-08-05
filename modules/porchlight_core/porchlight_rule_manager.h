#pragma once

#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#include "scene/main/node.h"

#include "porchlight_rule.h"

class PorchlightProgress;

class PorchlightRuleManager : public Node {
    GDCLASS(PorchlightRuleManager, Node);

    TypedArray<PorchlightRule> rules;

    bool run_on_ready = true;
    bool watch_progress = true;

    int max_evaluations_per_cycle = 64;

    bool processing_scheduled = false;
    bool is_processing = false;

    Vector<uint8_t> pending_rules;
    Vector<uint8_t> evaluated_rules;

    int last_evaluation_count = 0;
    int last_action_change_count = 0;
    bool last_cycle_limit_reached = false;

    PorchlightProgress *_get_progress() const;

    void _connect_rules();
    void _disconnect_rules();

    void _connect_progress();
    void _disconnect_progress();

    void _ensure_queue_size();
    void _clear_queue_state();

    void _queue_rule_index(int p_index);
    void _queue_all_rule_indexes();

    void _queue_rules_for_milestone(
            const StringName &p_milestone);

    int _find_next_queued_rule() const;
    int _count_pending_rules() const;

    void _schedule_processing();
    void _process_queued_rules();

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
    void set_rules(
            const TypedArray<PorchlightRule> &p_rules);

    TypedArray<PorchlightRule> get_rules() const;

    void set_run_on_ready(bool p_run_on_ready);
    bool is_run_on_ready() const;

    void set_watch_progress(bool p_watch_progress);
    bool is_watch_progress() const;

    void set_max_evaluations_per_cycle(
            int p_max_evaluations);

    int get_max_evaluations_per_cycle() const;

    void request_evaluation();

    void request_evaluation_for_milestone(
            const StringName &p_milestone);

    void clear_pending_evaluations();

    int get_pending_rule_count() const;
    bool is_processing_rules() const;

    int get_last_evaluation_count() const;
    int get_last_action_change_count() const;
    bool did_last_cycle_reach_limit() const;

    PackedStringArray get_setup_warnings() const;

    PackedStringArray get_configuration_warnings()
            const override;
};
