#include "porchlight_rule_manager.h"

#include "core/config/engine.h"
#include "core/error/error_macros.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"
#include "core/object/message_queue.h"

#include "porchlight_action.h"
#include "porchlight_condition.h"
#include "porchlight_progress.h"

PorchlightProgress *
PorchlightRuleManager::_get_progress() const {
    Object *singleton_object =
            Engine::get_singleton()
                    ->get_singleton_object(
                            "PorchlightProgress");

    return Object::cast_to<PorchlightProgress>(
            singleton_object);
}

void PorchlightRuleManager::_connect_rules() {
    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_rule_changed);

    for (int index = 0;
            index < rules.size();
            index++) {
        const Ref<PorchlightRule> current_rule =
                rules[index];

        if (current_rule.is_null()) {
            continue;
        }

        if (!current_rule->is_connected(
                    "changed",
                    callback)) {
            current_rule->connect(
                    "changed",
                    callback);
        }
    }
}

void PorchlightRuleManager::_disconnect_rules() {
    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_rule_changed);

    for (int index = 0;
            index < rules.size();
            index++) {
        const Ref<PorchlightRule> current_rule =
                rules[index];

        if (current_rule.is_null()) {
            continue;
        }

        if (current_rule->is_connected(
                    "changed",
                    callback)) {
            current_rule->disconnect(
                    "changed",
                    callback);
        }
    }
}

void PorchlightRuleManager::_connect_progress() {
    if (!watch_progress) {
        return;
    }

#ifdef TOOLS_ENABLED
    if (is_part_of_edited_scene()) {
        return;
    }
#endif

    PorchlightProgress *progress =
            _get_progress();

    if (progress == nullptr) {
        return;
    }

    const Callable completed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_milestone_completed);

    const Callable removed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_milestone_removed);

    const Callable cleared_callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_milestones_cleared);

    if (!progress->is_connected(
                "milestone_completed",
                completed_callback)) {
        progress->connect(
                "milestone_completed",
                completed_callback);
    }

    if (!progress->is_connected(
                "milestone_removed",
                removed_callback)) {
        progress->connect(
                "milestone_removed",
                removed_callback);
    }

    if (!progress->is_connected(
                "milestones_cleared",
                cleared_callback)) {
        progress->connect(
                "milestones_cleared",
                cleared_callback);
    }
}

void PorchlightRuleManager::_disconnect_progress() {
    PorchlightProgress *progress =
            _get_progress();

    if (progress == nullptr) {
        return;
    }

    const Callable completed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_milestone_completed);

    const Callable removed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_milestone_removed);

    const Callable cleared_callback =
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _on_milestones_cleared);

    if (progress->is_connected(
                "milestone_completed",
                completed_callback)) {
        progress->disconnect(
                "milestone_completed",
                completed_callback);
    }

    if (progress->is_connected(
                "milestone_removed",
                removed_callback)) {
        progress->disconnect(
                "milestone_removed",
                removed_callback);
    }

    if (progress->is_connected(
                "milestones_cleared",
                cleared_callback)) {
        progress->disconnect(
                "milestones_cleared",
                cleared_callback);
    }
}

void PorchlightRuleManager::_ensure_queue_size() {
    const int rule_count = rules.size();

    const int previous_pending_size =
            pending_rules.size();

    pending_rules.resize(rule_count);

    for (int index = previous_pending_size;
            index < rule_count;
            index++) {
        pending_rules.write[index] = 0;
    }

    const int previous_evaluated_size =
            evaluated_rules.size();

    evaluated_rules.resize(rule_count);

    for (int index = previous_evaluated_size;
            index < rule_count;
            index++) {
        evaluated_rules.write[index] = 0;
    }
}

void PorchlightRuleManager::_clear_queue_state() {
    _ensure_queue_size();

    for (int index = 0;
            index < pending_rules.size();
            index++) {
        pending_rules.write[index] = 0;
        evaluated_rules.write[index] = 0;
    }

    processing_scheduled = false;
}

void PorchlightRuleManager::_queue_rule_index(
        int p_index) {
    if (p_index < 0 ||
            p_index >= rules.size()) {
        return;
    }

    _ensure_queue_size();

    if (evaluated_rules[p_index] != 0) {
        return;
    }

    pending_rules.write[p_index] = 1;

    _schedule_processing();
}

void PorchlightRuleManager::_queue_all_rule_indexes() {
    _ensure_queue_size();

    for (int index = 0;
            index < rules.size();
            index++) {
        if (evaluated_rules[index] != 0) {
            continue;
        }

        pending_rules.write[index] = 1;
    }

    _schedule_processing();
}

void PorchlightRuleManager::_queue_rules_for_milestone(
        const StringName &p_milestone) {
    for (int index = 0;
            index < rules.size();
            index++) {
        const Ref<PorchlightRule> current_rule =
                rules[index];

        if (current_rule.is_null()) {
            continue;
        }

        const Ref<PorchlightCondition> condition =
                current_rule->get_condition();

        if (condition.is_null()) {
            continue;
        }

        if (!condition->references_milestone(
                    p_milestone)) {
            continue;
        }

        _queue_rule_index(index);
    }
}

int PorchlightRuleManager::
_find_next_queued_rule() const {
    for (int index = 0;
            index < pending_rules.size();
            index++) {
        if (pending_rules[index] != 0) {
            return index;
        }
    }

    return -1;
}

int PorchlightRuleManager::
_count_pending_rules() const {
    int pending_count = 0;

    for (int index = 0;
            index < pending_rules.size();
            index++) {
        if (pending_rules[index] != 0) {
            pending_count++;
        }
    }

    return pending_count;
}

void PorchlightRuleManager::_schedule_processing() {
    if (is_processing ||
            processing_scheduled ||
            _count_pending_rules() == 0) {
        return;
    }

    processing_scheduled = true;

    MessageQueue::get_singleton()->push_callable(
            callable_mp(
                    this,
                    &PorchlightRuleManager::
                            _process_queued_rules));
}

void PorchlightRuleManager::_process_queued_rules() {
    processing_scheduled = false;

    if (is_processing) {
        return;
    }

    _ensure_queue_size();

    if (_count_pending_rules() == 0) {
        return;
    }

    is_processing = true;

    last_evaluation_count = 0;
    last_action_change_count = 0;
    last_cycle_limit_reached = false;

    for (int index = 0;
            index < evaluated_rules.size();
            index++) {
        evaluated_rules.write[index] = 0;
    }

    while (true) {
        const int rule_index =
                _find_next_queued_rule();

        if (rule_index < 0) {
            break;
        }

        if (last_evaluation_count >=
                max_evaluations_per_cycle) {
            last_cycle_limit_reached = true;
            break;
        }

        pending_rules.write[rule_index] = 0;
        evaluated_rules.write[rule_index] = 1;

        bool condition_met = false;
        bool action_changed = false;

        const Ref<PorchlightRule> current_rule =
                rules[rule_index];

        if (current_rule.is_valid() &&
                current_rule->is_valid()) {
            condition_met =
                    current_rule->
                            is_condition_met();

            if (condition_met) {
                action_changed =
                        current_rule->
                                evaluate_and_execute();
            }
        }

        last_evaluation_count++;

        if (action_changed) {
            last_action_change_count++;
        }

        emit_signal(
                "rule_evaluated",
                rule_index,
                condition_met,
                action_changed);
    }

    const int pending_rule_count =
            _count_pending_rules();

    for (int index = 0;
            index < pending_rules.size();
            index++) {
        pending_rules.write[index] = 0;
        evaluated_rules.write[index] = 0;
    }

    is_processing = false;

    if (last_cycle_limit_reached) {
        emit_signal(
                "cycle_limit_reached",
                max_evaluations_per_cycle,
                pending_rule_count);
    }

    emit_signal(
            "cycle_completed",
            last_evaluation_count,
            last_action_change_count,
            last_cycle_limit_reached);
}

void PorchlightRuleManager::_on_rule_changed() {
    update_configuration_warnings();

    if (!is_inside_tree()) {
        return;
    }

#ifdef TOOLS_ENABLED
    if (is_part_of_edited_scene()) {
        return;
    }
#endif

    request_evaluation();
}

void PorchlightRuleManager::_on_milestone_completed(
        const StringName &p_milestone) {
    request_evaluation_for_milestone(
            p_milestone);
}

void PorchlightRuleManager::_on_milestone_removed(
        const StringName &p_milestone) {
    request_evaluation_for_milestone(
            p_milestone);
}

void PorchlightRuleManager::_on_milestones_cleared() {
    request_evaluation();
}

void PorchlightRuleManager::_notification(
        int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
            _connect_rules();
            _connect_progress();
        } break;

        case NOTIFICATION_READY: {
            if (!run_on_ready) {
                return;
            }

#ifdef TOOLS_ENABLED
            if (is_part_of_edited_scene()) {
                return;
            }
#endif

            request_evaluation();
        } break;

        case NOTIFICATION_EXIT_TREE: {
            _disconnect_progress();
            clear_pending_evaluations();
        } break;
    }
}

void PorchlightRuleManager::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("set_rules", "rules"),
            &PorchlightRuleManager::set_rules);

    ClassDB::bind_method(
            D_METHOD("get_rules"),
            &PorchlightRuleManager::get_rules);

    ClassDB::bind_method(
            D_METHOD(
                    "set_run_on_ready",
                    "run_on_ready"),
            &PorchlightRuleManager::
                    set_run_on_ready);

    ClassDB::bind_method(
            D_METHOD("is_run_on_ready"),
            &PorchlightRuleManager::
                    is_run_on_ready);

    ClassDB::bind_method(
            D_METHOD(
                    "set_watch_progress",
                    "watch_progress"),
            &PorchlightRuleManager::
                    set_watch_progress);

    ClassDB::bind_method(
            D_METHOD("is_watch_progress"),
            &PorchlightRuleManager::
                    is_watch_progress);

    ClassDB::bind_method(
            D_METHOD(
                    "set_max_evaluations_per_cycle",
                    "max_evaluations"),
            &PorchlightRuleManager::
                    set_max_evaluations_per_cycle);

    ClassDB::bind_method(
            D_METHOD(
                    "get_max_evaluations_per_cycle"),
            &PorchlightRuleManager::
                    get_max_evaluations_per_cycle);

    ClassDB::bind_method(
            D_METHOD("request_evaluation"),
            &PorchlightRuleManager::
                    request_evaluation);

    ClassDB::bind_method(
            D_METHOD(
                    "request_evaluation_for_milestone",
                    "milestone"),
            &PorchlightRuleManager::
                    request_evaluation_for_milestone);

    ClassDB::bind_method(
            D_METHOD("clear_pending_evaluations"),
            &PorchlightRuleManager::
                    clear_pending_evaluations);

    ClassDB::bind_method(
            D_METHOD("get_pending_rule_count"),
            &PorchlightRuleManager::
                    get_pending_rule_count);

    ClassDB::bind_method(
            D_METHOD("is_processing_rules"),
            &PorchlightRuleManager::
                    is_processing_rules);

    ClassDB::bind_method(
            D_METHOD("get_last_evaluation_count"),
            &PorchlightRuleManager::
                    get_last_evaluation_count);

    ClassDB::bind_method(
            D_METHOD(
                    "get_last_action_change_count"),
            &PorchlightRuleManager::
                    get_last_action_change_count);

    ClassDB::bind_method(
            D_METHOD(
                    "did_last_cycle_reach_limit"),
            &PorchlightRuleManager::
                    did_last_cycle_reach_limit);

    ClassDB::bind_method(
            D_METHOD("get_setup_warnings"),
            &PorchlightRuleManager::
                    get_setup_warnings);

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::ARRAY,
                    "rules",
                    PROPERTY_HINT_ARRAY_TYPE,
                    MAKE_RESOURCE_TYPE_HINT(
                            "PorchlightRule")),
            "set_rules",
            "get_rules");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::BOOL,
                    "run_on_ready"),
            "set_run_on_ready",
            "is_run_on_ready");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::BOOL,
                    "watch_progress"),
            "set_watch_progress",
            "is_watch_progress");

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::INT,
                    "max_evaluations_per_cycle",
                    PROPERTY_HINT_RANGE,
                    "1,4096,1,or_greater"),
            "set_max_evaluations_per_cycle",
            "get_max_evaluations_per_cycle");

    ADD_SIGNAL(
            MethodInfo(
                    "rule_evaluated",
                    PropertyInfo(
                            Variant::INT,
                            "rule_index"),
                    PropertyInfo(
                            Variant::BOOL,
                            "condition_met"),
                    PropertyInfo(
                            Variant::BOOL,
                            "action_changed")));

    ADD_SIGNAL(
            MethodInfo(
                    "cycle_completed",
                    PropertyInfo(
                            Variant::INT,
                            "evaluation_count"),
                    PropertyInfo(
                            Variant::INT,
                            "action_change_count"),
                    PropertyInfo(
                            Variant::BOOL,
                            "limit_reached")));

    ADD_SIGNAL(
            MethodInfo(
                    "cycle_limit_reached",
                    PropertyInfo(
                            Variant::INT,
                            "evaluation_limit"),
                    PropertyInfo(
                            Variant::INT,
                            "pending_rule_count")));
}

void PorchlightRuleManager::set_rules(
        const TypedArray<PorchlightRule> &p_rules) {
    ERR_FAIL_COND_MSG(
            is_processing,
            "PorchlightRuleManager cannot replace its "
            "rules while processing a cycle.");

    _disconnect_rules();

    rules = p_rules;

    _clear_queue_state();
    _connect_rules();

    update_configuration_warnings();

    if (!is_inside_tree()) {
        return;
    }

#ifdef TOOLS_ENABLED
    if (is_part_of_edited_scene()) {
        return;
    }
#endif

    request_evaluation();
}

TypedArray<PorchlightRule>
PorchlightRuleManager::get_rules() const {
    return rules;
}

void PorchlightRuleManager::set_run_on_ready(
        bool p_run_on_ready) {
    run_on_ready = p_run_on_ready;
}

bool PorchlightRuleManager::is_run_on_ready() const {
    return run_on_ready;
}

void PorchlightRuleManager::set_watch_progress(
        bool p_watch_progress) {
    if (watch_progress == p_watch_progress) {
        return;
    }

    watch_progress = p_watch_progress;

    if (!is_inside_tree()) {
        return;
    }

    if (watch_progress) {
        _connect_progress();

#ifdef TOOLS_ENABLED
        if (is_part_of_edited_scene()) {
            return;
        }
#endif

        request_evaluation();
    } else {
        _disconnect_progress();
    }
}

bool PorchlightRuleManager::
is_watch_progress() const {
    return watch_progress;
}

void PorchlightRuleManager::
set_max_evaluations_per_cycle(
        int p_max_evaluations) {
    const int normalized_limit =
            p_max_evaluations < 1
                    ? 1
                    : p_max_evaluations;

    if (max_evaluations_per_cycle ==
            normalized_limit) {
        return;
    }

    max_evaluations_per_cycle =
            normalized_limit;

    update_configuration_warnings();
}

int PorchlightRuleManager::
get_max_evaluations_per_cycle() const {
    return max_evaluations_per_cycle;
}

void PorchlightRuleManager::request_evaluation() {
    _queue_all_rule_indexes();
}

void PorchlightRuleManager::
request_evaluation_for_milestone(
        const StringName &p_milestone) {
    const String milestone_text =
            String(p_milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return;
    }

    _queue_rules_for_milestone(
            StringName(milestone_text));
}

void PorchlightRuleManager::
clear_pending_evaluations() {
    _ensure_queue_size();

    for (int index = 0;
            index < pending_rules.size();
            index++) {
        pending_rules.write[index] = 0;
    }

    processing_scheduled = false;
}

int PorchlightRuleManager::
get_pending_rule_count() const {
    return _count_pending_rules();
}

bool PorchlightRuleManager::
is_processing_rules() const {
    return is_processing;
}

int PorchlightRuleManager::
get_last_evaluation_count() const {
    return last_evaluation_count;
}

int PorchlightRuleManager::
get_last_action_change_count() const {
    return last_action_change_count;
}

bool PorchlightRuleManager::
did_last_cycle_reach_limit() const {
    return last_cycle_limit_reached;
}

PackedStringArray
PorchlightRuleManager::get_setup_warnings() const {
    PackedStringArray warnings;

    if (rules.is_empty()) {
        warnings.push_back(
                "Add at least one PorchlightRule "
                "to this manager.");

        return warnings;
    }

    for (int index = 0;
            index < rules.size();
            index++) {
        const Ref<PorchlightRule> current_rule =
                rules[index];

        const String entry_name =
                String("Rule entry ") +
                itos(index + 1);

        if (current_rule.is_null()) {
            warnings.push_back(
                    entry_name +
                    " is empty.");

            continue;
        }

        const Ref<PorchlightCondition> condition =
                current_rule->get_condition();

        if (condition.is_null()) {
            warnings.push_back(
                    entry_name +
                    " needs a PorchlightCondition.");
        } else if (!condition->is_valid()) {
            warnings.push_back(
                    entry_name +
                    " condition needs at least "
                    "one milestone.");
        }

        const Ref<PorchlightAction> action =
                current_rule->get_action();

        if (action.is_null()) {
            warnings.push_back(
                    entry_name +
                    " needs a PorchlightAction.");
        } else if (!action->is_valid()) {
            warnings.push_back(
                    entry_name +
                    " action needs a milestone.");
        }
    }

    if (max_evaluations_per_cycle <
            rules.size()) {
        warnings.push_back(
                "The cycle evaluation limit is lower "
                "than the number of configured rules.");
    }

    return warnings;
}

PackedStringArray
PorchlightRuleManager::
get_configuration_warnings() const {
    PackedStringArray warnings =
            Node::get_configuration_warnings();

    const PackedStringArray setup_warnings =
            get_setup_warnings();

    for (int index = 0;
            index < setup_warnings.size();
            index++) {
        warnings.push_back(
                setup_warnings[index]);
    }

    return warnings;
}
