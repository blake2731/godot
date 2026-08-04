#include "porchlight_rule_runner.h"

#include "core/config/engine.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

#include "porchlight_action.h"
#include "porchlight_condition.h"
#include "porchlight_progress.h"
#include "porchlight_rule.h"

PorchlightProgress *
PorchlightRuleRunner::_get_progress() const {
    Object *singleton_object =
            Engine::get_singleton()
                    ->get_singleton_object(
                            "PorchlightProgress");

    return Object::cast_to<PorchlightProgress>(
            singleton_object);
}

void PorchlightRuleRunner::_connect_rule() {
    if (rule.is_null()) {
        return;
    }

    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
                            _on_rule_changed);

    if (!rule->is_connected(
                "changed",
                callback)) {
        rule->connect(
                "changed",
                callback);
    }
}

void PorchlightRuleRunner::_disconnect_rule() {
    if (rule.is_null()) {
        return;
    }

    const Callable callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
                            _on_rule_changed);

    if (rule->is_connected(
                "changed",
                callback)) {
        rule->disconnect(
                "changed",
                callback);
    }
}

void PorchlightRuleRunner::_connect_progress() {
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
                    &PorchlightRuleRunner::
                            _on_milestone_completed);

    const Callable removed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
                            _on_milestone_removed);

    const Callable cleared_callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
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

void PorchlightRuleRunner::_disconnect_progress() {
    PorchlightProgress *progress =
            _get_progress();

    if (progress == nullptr) {
        return;
    }

    const Callable completed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
                            _on_milestone_completed);

    const Callable removed_callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
                            _on_milestone_removed);

    const Callable cleared_callback =
            callable_mp(
                    this,
                    &PorchlightRuleRunner::
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

void PorchlightRuleRunner::_evaluate_for_milestone(
        const StringName &p_milestone) {
    if (rule.is_null()) {
        return;
    }

    const Ref<PorchlightCondition> condition =
            rule->get_condition();

    if (condition.is_null()) {
        return;
    }

    if (condition->get_milestone() != p_milestone) {
        return;
    }

    evaluate_rule();
}

void PorchlightRuleRunner::_on_rule_changed() {
    has_run = false;
    update_configuration_warnings();
}

void PorchlightRuleRunner::_on_milestone_completed(
        const StringName &p_milestone) {
    _evaluate_for_milestone(p_milestone);
}

void PorchlightRuleRunner::_on_milestone_removed(
        const StringName &p_milestone) {
    _evaluate_for_milestone(p_milestone);
}

void PorchlightRuleRunner::_on_milestones_cleared() {
    evaluate_rule();
}

void PorchlightRuleRunner::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
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

            evaluate_rule();
        } break;

        case NOTIFICATION_EXIT_TREE: {
            _disconnect_progress();
        } break;
    }
}

void PorchlightRuleRunner::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("set_rule", "rule"),
            &PorchlightRuleRunner::set_rule);

    ClassDB::bind_method(
            D_METHOD("get_rule"),
            &PorchlightRuleRunner::get_rule);

    ClassDB::bind_method(
            D_METHOD(
                    "set_run_on_ready",
                    "run_on_ready"),
            &PorchlightRuleRunner::set_run_on_ready);

    ClassDB::bind_method(
            D_METHOD("is_run_on_ready"),
            &PorchlightRuleRunner::is_run_on_ready);

    ClassDB::bind_method(
            D_METHOD(
                    "set_watch_progress",
                    "watch_progress"),
            &PorchlightRuleRunner::set_watch_progress);

    ClassDB::bind_method(
            D_METHOD("is_watch_progress"),
            &PorchlightRuleRunner::is_watch_progress);

    ClassDB::bind_method(
            D_METHOD("set_run_once", "run_once"),
            &PorchlightRuleRunner::set_run_once);

    ClassDB::bind_method(
            D_METHOD("is_run_once"),
            &PorchlightRuleRunner::is_run_once);

    ClassDB::bind_method(
            D_METHOD("has_run_rule"),
            &PorchlightRuleRunner::has_run_rule);

    ClassDB::bind_method(
            D_METHOD("evaluate_rule"),
            &PorchlightRuleRunner::evaluate_rule);

    ClassDB::bind_method(
            D_METHOD("reset_runner"),
            &PorchlightRuleRunner::reset_runner);

    ClassDB::bind_method(
            D_METHOD("get_description"),
            &PorchlightRuleRunner::get_description);

    ClassDB::bind_method(
            D_METHOD("get_setup_warnings"),
            &PorchlightRuleRunner::get_setup_warnings);

    ADD_PROPERTY(
            PropertyInfo(
                    Variant::OBJECT,
                    "rule",
                    PROPERTY_HINT_RESOURCE_TYPE,
                    "PorchlightRule"),
            "set_rule",
            "get_rule");

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
                    Variant::BOOL,
                    "run_once"),
            "set_run_once",
            "is_run_once");

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

void PorchlightRuleRunner::set_rule(
        const Ref<PorchlightRule> &p_rule) {
    if (rule == p_rule) {
        return;
    }

    _disconnect_rule();

    rule = p_rule;
    has_run = false;

    _connect_rule();
    update_configuration_warnings();
}

Ref<PorchlightRule>
PorchlightRuleRunner::get_rule() const {
    return rule;
}

void PorchlightRuleRunner::set_run_on_ready(
        bool p_run_on_ready) {
    run_on_ready = p_run_on_ready;
}

bool PorchlightRuleRunner::is_run_on_ready() const {
    return run_on_ready;
}

void PorchlightRuleRunner::set_watch_progress(
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

        evaluate_rule();
    } else {
        _disconnect_progress();
    }
}

bool PorchlightRuleRunner::is_watch_progress() const {
    return watch_progress;
}

void PorchlightRuleRunner::set_run_once(
        bool p_run_once) {
    run_once = p_run_once;
}

bool PorchlightRuleRunner::is_run_once() const {
    return run_once;
}

bool PorchlightRuleRunner::has_run_rule() const {
    return has_run;
}

bool PorchlightRuleRunner::evaluate_rule() {
    if (is_evaluating) {
        return false;
    }

    if (run_once && has_run) {
        return false;
    }

    is_evaluating = true;

    bool condition_met = false;
    bool action_changed = false;

    if (rule.is_valid() && rule->is_valid()) {
        condition_met =
                rule->is_condition_met();

        if (run_once && condition_met) {
            has_run = true;
        }

        if (condition_met) {
            action_changed =
                    rule->evaluate_and_execute();
        }
    }

    emit_signal(
            "evaluated",
            condition_met,
            action_changed);

    is_evaluating = false;

    return action_changed;
}

void PorchlightRuleRunner::reset_runner() {
    has_run = false;
}

String PorchlightRuleRunner::get_description() const {
    if (rule.is_null()) {
        return "No rule assigned.";
    }

    if (run_once && has_run) {
        return String("Rule already ran: ") +
                rule->get_description();
    }

    return rule->get_description();
}

PackedStringArray
PorchlightRuleRunner::get_setup_warnings() const {
    PackedStringArray warnings;

    if (rule.is_null()) {
        warnings.push_back(
                "Assign a PorchlightRule to this runner.");

        return warnings;
    }

    const Ref<PorchlightCondition> condition =
            rule->get_condition();

    if (condition.is_null()) {
        warnings.push_back(
                "The assigned rule needs a "
                "PorchlightCondition.");
    } else if (
            String(condition->get_milestone())
                    .strip_edges()
                    .is_empty()) {
        warnings.push_back(
                "The assigned condition needs a "
                "milestone.");
    }

    const Ref<PorchlightAction> action =
            rule->get_action();

    if (action.is_null()) {
        warnings.push_back(
                "The assigned rule needs a "
                "PorchlightAction.");
    } else if (!action->is_valid()) {
        warnings.push_back(
                "The assigned action needs a milestone.");
    }

    return warnings;
}

PackedStringArray
PorchlightRuleRunner::get_configuration_warnings()
        const {
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
