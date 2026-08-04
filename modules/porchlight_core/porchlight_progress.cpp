#include "porchlight_progress.h"

#include "core/error/error_macros.h"
#include "core/io/config_file.h"
#include "core/object/class_db.h"

namespace {

constexpr char SAVE_PATH[] =
        "user://porchlight_progress.cfg";

constexpr char SAVE_SECTION[] =
        "progress";

constexpr char SAVE_KEY[] =
        "completed_milestones";

} // namespace

PorchlightProgress::PorchlightProgress() {
    const Error load_error = load_progress();

    if (load_error != OK) {
        completed_lookup.clear();
        completed_milestones.clear();

        ERR_PRINT(
                "PorchlightProgress could not load "
                "the saved milestone data.");
    }
}

void PorchlightProgress::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD(
                    "complete_milestone",
                    "milestone"),
            &PorchlightProgress::
                    complete_milestone);

    ClassDB::bind_method(
            D_METHOD(
                    "remove_milestone",
                    "milestone"),
            &PorchlightProgress::
                    remove_milestone);

    ClassDB::bind_method(
            D_METHOD(
                    "is_complete",
                    "milestone"),
            &PorchlightProgress::is_complete);

    ClassDB::bind_method(
            D_METHOD(
                    "get_completed_milestones"),
            &PorchlightProgress::
                    get_completed_milestones);

    ClassDB::bind_method(
            D_METHOD("get_completed_count"),
            &PorchlightProgress::
                    get_completed_count);

    ClassDB::bind_method(
            D_METHOD("save_progress"),
            &PorchlightProgress::save_progress);

    ClassDB::bind_method(
            D_METHOD("load_progress"),
            &PorchlightProgress::load_progress);

    ClassDB::bind_method(
            D_METHOD("get_save_path"),
            &PorchlightProgress::get_save_path);

    ClassDB::bind_method(
            D_METHOD("clear_milestones"),
            &PorchlightProgress::
                    clear_milestones);

    ADD_SIGNAL(
            MethodInfo(
                    "milestone_completed",
                    PropertyInfo(
                            Variant::STRING_NAME,
                            "milestone")));

    ADD_SIGNAL(
            MethodInfo(
                    "milestone_removed",
                    PropertyInfo(
                            Variant::STRING_NAME,
                            "milestone")));

    ADD_SIGNAL(
            MethodInfo("milestones_cleared"));
}

bool PorchlightProgress::complete_milestone(
        const StringName &p_milestone) {
    const String milestone_text =
            String(p_milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return false;
    }

    const StringName milestone =
            milestone_text;

    if (completed_lookup.has(milestone)) {
        return false;
    }

    completed_lookup.insert(milestone);
    completed_milestones.push_back(milestone);

    const Error save_error =
            save_progress();

    if (save_error != OK) {
        ERR_PRINT(
                "PorchlightProgress could not save "
                "the completed milestone.");
    }

    emit_signal(
            "milestone_completed",
            milestone);

    return true;
}

bool PorchlightProgress::remove_milestone(
        const StringName &p_milestone) {
    const String milestone_text =
            String(p_milestone).strip_edges();

    if (milestone_text.is_empty()) {
        return false;
    }

    const StringName milestone =
            milestone_text;

    if (!completed_lookup.has(milestone)) {
        return false;
    }

    completed_lookup.erase(milestone);
    completed_milestones.erase(milestone);

    const Error save_error =
            save_progress();

    if (save_error != OK) {
        ERR_PRINT(
                "PorchlightProgress could not save "
                "the removed milestone.");
    }

    emit_signal(
            "milestone_removed",
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

Array PorchlightProgress::
get_completed_milestones() const {
    Array milestones;

    milestones.resize(
            completed_milestones.size());

    for (int index = 0;
            index < completed_milestones.size();
            index++) {
        milestones[index] =
                completed_milestones[index];
    }

    return milestones;
}

int PorchlightProgress::
get_completed_count() const {
    return completed_milestones.size();
}

Error PorchlightProgress::save_progress() {
    Ref<ConfigFile> config;
    config.instantiate();

    Array saved_milestones;

    saved_milestones.resize(
            completed_milestones.size());

    for (int index = 0;
            index < completed_milestones.size();
            index++) {
        saved_milestones[index] =
                String(
                        completed_milestones[
                                index]);
    }

    config->set_value(
            SAVE_SECTION,
            SAVE_KEY,
            saved_milestones);

    return config->save(SAVE_PATH);
}

Error PorchlightProgress::load_progress() {
    Ref<ConfigFile> config;
    config.instantiate();

    const Error load_error =
            config->load(SAVE_PATH);

    if (load_error == ERR_FILE_NOT_FOUND) {
        completed_lookup.clear();
        completed_milestones.clear();

        return OK;
    }

    if (load_error != OK) {
        return load_error;
    }

    const Variant saved_value =
            config->get_value(
                    SAVE_SECTION,
                    SAVE_KEY,
                    Array());

    if (saved_value.get_type() !=
            Variant::ARRAY) {
        return ERR_INVALID_DATA;
    }

    const Array saved_milestones =
            saved_value;

    completed_lookup.clear();
    completed_milestones.clear();

    for (int index = 0;
            index < saved_milestones.size();
            index++) {
        const Variant entry =
                saved_milestones[index];

        if (entry.get_type() !=
                        Variant::STRING &&
                entry.get_type() !=
                        Variant::STRING_NAME) {
            continue;
        }

        const String milestone_text =
                String(entry).strip_edges();

        if (milestone_text.is_empty()) {
            continue;
        }

        const StringName milestone =
                milestone_text;

        if (completed_lookup.has(
                    milestone)) {
            continue;
        }

        completed_lookup.insert(
                milestone);

        completed_milestones.push_back(
                milestone);
    }

    return OK;
}

String PorchlightProgress::
get_save_path() const {
    return SAVE_PATH;
}

void PorchlightProgress::clear_milestones() {
    const bool had_milestones =
            !completed_milestones.is_empty();

    completed_lookup.clear();
    completed_milestones.clear();

    const Error save_error =
            save_progress();

    if (save_error != OK) {
        ERR_PRINT(
                "PorchlightProgress could not save "
                "the cleared milestone data.");
    }

    if (had_milestones) {
        emit_signal(
                "milestones_cleared");
    }
}
