#include "porchlight_rule_runner.h"

#include "core/object/class_db.h"

#include "porchlight_action.h"
#include "porchlight_condition.h"
#include "porchlight_rule.h"

void PorchlightRuleRunner::_notification(int p_what) {
    if (p_what != NOTIFICATION_READY ||
            !run_on_ready) {
        return;
    }

#ifdef TOOLS_ENABLED
    if (is_part_of_edited_scene()) {
        return;
    }
#endif

    evaluate_rule();
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

    rule = p_rule;
    has_run = false;

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
    if (run_once && has_run) {
        return false;
    }

    if (rule.is_null() || !rule->is_valid()) {
        emit_signal(
                "evaluated",
                false,
                false);

        return false;
    }

    const bool condition_met =
            rule->is_condition_met();

    const bool action_changed =
            rule->evaluate_and_execute();

    if (condition_met) {
        has_run = true;
    }

    emit_signal(
            "evaluated",
            condition_met,
            action_changed);

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
