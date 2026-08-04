#include "porchlight_progress.h"

#include "core/object/class_db.h"

void PorchlightProgress::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("complete_milestone", "milestone"),
            &PorchlightProgress::complete_milestone);

    ClassDB::bind_method(
            D_METHOD("is_complete", "milestone"),
            &PorchlightProgress::is_complete);

    ClassDB::bind_method(
            D_METHOD("get_completed_milestones"),
            &PorchlightProgress::get_completed_milestones);

    ClassDB::bind_method(
            D_METHOD("get_completed_count"),
            &PorchlightProgress::get_completed_count);

    ClassDB::bind_method(
            D_METHOD("clear_milestones"),
            &PorchlightProgress::clear_milestones);

    ADD_SIGNAL(
            MethodInfo(
                    "milestone_completed",
                    PropertyInfo(
                            Variant::STRING_NAME,
                            "milestone")));

    ADD_SIGNAL(MethodInfo("milestones_cleared"));
}

bool PorchlightProgress::complete_milestone(
        const StringName &p_milestone) {
    const String milestone_text =
            String(p_milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return false;
    }

    const StringName milestone = milestone_text;

    if (completed_lookup.has(milestone)) {
        return false;
    }

    completed_lookup.insert(milestone);
    completed_milestones.push_back(milestone);

    emit_signal(
            "milestone_completed",
            milestone);

    return true;
}

bool PorchlightProgress::is_complete(
        const StringName &p_milestone) const {
    const String milestone_text =
            String(p_milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return false;
    }

    return completed_lookup.has(
            StringName(milestone_text));
}

Array PorchlightProgress::get_completed_milestones() const {
    Array milestones;
    milestones.resize(completed_milestones.size());

    for (int index = 0;
            index < completed_milestones.size();
            index++) {
        milestones[index] =
                completed_milestones[index];
    }

    return milestones;
}

int PorchlightProgress::get_completed_count() const {
    return completed_milestones.size();
}

void PorchlightProgress::clear_milestones() {
    if (completed_milestones.is_empty()) {
        return;
    }

    completed_lookup.clear();
    completed_milestones.clear();

    emit_signal("milestones_cleared");
}
