#include "porchlight_progress.h"

#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/io/config_file.h"
#include "core/io/dir_access.h"
#include "core/object/class_db.h"

namespace {

constexpr char DEFAULT_SAVE_PATH[] =
        "user://porchlight_progress.cfg";

constexpr char SAVE_SECTION[] =
        "progress";

constexpr char SAVE_VERSION_KEY[] =
        "format_version";

constexpr char SAVE_KEY[] =
        "completed_milestones";

constexpr int LEGACY_SAVE_FORMAT_VERSION = 0;
constexpr int CURRENT_SAVE_FORMAT_VERSION = 1;

} // namespace

PackedStringArray
PorchlightProgress::_normalize_milestones(
        const PackedStringArray &p_milestones)
        const {
    PackedStringArray normalized_milestones;

    for (int index = 0;
            index < p_milestones.size();
            index++) {
        const String normalized_text =
                p_milestones[index].strip_edges();

        if (normalized_text.is_empty()) {
            continue;
        }

        if (normalized_milestones.has(
                    normalized_text)) {
            continue;
        }

        normalized_milestones.push_back(
                normalized_text);
    }

    return normalized_milestones;
}

String PorchlightProgress::_normalize_save_path(
        const String &p_save_path) const {
    String normalized_path =
            p_save_path.strip_edges();

    normalized_path =
            normalized_path.replace(
                    "\\",
                    "/");

    if (!normalized_path.begins_with(
                "user://")) {
        return String();
    }

    const String relative_path =
            normalized_path.substr(7);

    if (relative_path.is_empty() ||
            relative_path.ends_with("/")) {
        return String();
    }

    const PackedStringArray path_parts =
            relative_path.split(
                    "/",
                    false);

    if (path_parts.is_empty()) {
        return String();
    }

    for (int index = 0;
            index < path_parts.size();
            index++) {
        const String path_part =
                path_parts[index];

        if (path_part == "." ||
                path_part == ".." ||
                !path_part.is_valid_filename()) {
            return String();
        }
    }

    normalized_path =
            normalized_path.simplify_path();

    if (!normalized_path.begins_with(
                "user://") ||
            normalized_path.get_file().is_empty()) {
        return String();
    }

    return normalized_path;
}

Error PorchlightProgress::_read_progress(
        const String &p_save_path,
        HashSet<StringName> &r_loaded_lookup,
        Vector<StringName> &r_loaded_milestones)
        const {
    r_loaded_lookup.clear();
    r_loaded_milestones.clear();

    Ref<ConfigFile> config;
    config.instantiate();

    const Error load_error =
            config->load(p_save_path);

    if (load_error == ERR_FILE_NOT_FOUND) {
        return OK;
    }

    if (load_error != OK) {
        return load_error;
    }

    int save_format_version =
            LEGACY_SAVE_FORMAT_VERSION;

    if (config->has_section_key(
                SAVE_SECTION,
                SAVE_VERSION_KEY)) {
        const Variant saved_version =
                config->get_value(
                        SAVE_SECTION,
                        SAVE_VERSION_KEY);

        if (saved_version.get_type() !=
                Variant::INT) {
            return ERR_INVALID_DATA;
        }

        const int64_t stored_version =
                saved_version;

        if (stored_version <
                LEGACY_SAVE_FORMAT_VERSION) {
            return ERR_INVALID_DATA;
        }

        if (stored_version >
                CURRENT_SAVE_FORMAT_VERSION) {
            return ERR_FILE_UNRECOGNIZED;
        }

        save_format_version =
                static_cast<int>(
                        stored_version);
    }

    if (save_format_version !=
                    LEGACY_SAVE_FORMAT_VERSION &&
            save_format_version !=
                    CURRENT_SAVE_FORMAT_VERSION) {
        return ERR_FILE_UNRECOGNIZED;
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

        if (r_loaded_lookup.has(
                    milestone)) {
            continue;
        }

        r_loaded_lookup.insert(
                milestone);

        r_loaded_milestones.push_back(
                milestone);
    }

    return OK;
}

void PorchlightProgress::_replace_progress_state(
        const HashSet<StringName> &p_loaded_lookup,
        const Vector<StringName>
                &p_loaded_milestones,
        bool p_emit_reload_signal) {
    PackedStringArray added_milestones;
    PackedStringArray removed_milestones;

    for (int index = 0;
            index < p_loaded_milestones.size();
            index++) {
        const StringName milestone =
                p_loaded_milestones[index];

        if (completed_lookup.has(
                    milestone)) {
            continue;
        }

        added_milestones.push_back(
                String(milestone));
    }

    for (int index = 0;
            index < completed_milestones.size();
            index++) {
        const StringName milestone =
                completed_milestones[index];

        if (p_loaded_lookup.has(
                    milestone)) {
            continue;
        }

        removed_milestones.push_back(
                String(milestone));
    }

    completed_lookup.clear();
    completed_milestones.clear();

    for (int index = 0;
            index < p_loaded_milestones.size();
            index++) {
        const StringName milestone =
                p_loaded_milestones[index];

        completed_lookup.insert(
                milestone);

        completed_milestones.push_back(
                milestone);
    }

    if (p_emit_reload_signal &&
            (!added_milestones.is_empty() ||
                    !removed_milestones.is_empty())) {
        emit_signal(
                "progress_reloaded",
                added_milestones,
                removed_milestones);
    }
}

Error PorchlightProgress::_load_progress(
        bool p_emit_reload_signal) {
    HashSet<StringName> loaded_lookup;
    Vector<StringName> loaded_milestones;

    const Error load_error =
            _read_progress(
                    save_path,
                    loaded_lookup,
                    loaded_milestones);

    if (load_error != OK) {
        return load_error;
    }

    _replace_progress_state(
            loaded_lookup,
            loaded_milestones,
            p_emit_reload_signal);

    return OK;
}

PorchlightProgress::PorchlightProgress() {
    save_path = DEFAULT_SAVE_PATH;

    const Error load_error =
            _load_progress(false);

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
                    "complete_milestones",
                    "milestones"),
            &PorchlightProgress::
                    complete_milestones);

    ClassDB::bind_method(
            D_METHOD(
                    "remove_milestone",
                    "milestone"),
            &PorchlightProgress::
                    remove_milestone);

    ClassDB::bind_method(
            D_METHOD(
                    "remove_milestones",
                    "milestones"),
            &PorchlightProgress::
                    remove_milestones);

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
            D_METHOD(
                    "set_save_path",
                    "save_path"),
            &PorchlightProgress::set_save_path);

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

    ADD_SIGNAL(
            MethodInfo(
                    "progress_reloaded",
                    PropertyInfo(
                            Variant::PACKED_STRING_ARRAY,
                            "added_milestones"),
                    PropertyInfo(
                            Variant::PACKED_STRING_ARRAY,
                            "removed_milestones")));

    ADD_SIGNAL(
            MethodInfo(
                    "save_path_changed",
                    PropertyInfo(
                            Variant::STRING,
                            "previous_path"),
                    PropertyInfo(
                            Variant::STRING,
                            "new_path")));
}

bool PorchlightProgress::complete_milestone(
        const StringName &p_milestone) {
    PackedStringArray requested_milestones;

    requested_milestones.push_back(
            String(p_milestone));

    return !complete_milestones(
                    requested_milestones)
                    .is_empty();
}

PackedStringArray
PorchlightProgress::complete_milestones(
        const PackedStringArray &p_milestones) {
    const PackedStringArray
            normalized_milestones =
                    _normalize_milestones(
                            p_milestones);

    PackedStringArray changed_milestones;

    for (int index = 0;
            index < normalized_milestones.size();
            index++) {
        const StringName milestone =
                normalized_milestones[index];

        if (completed_lookup.has(
                    milestone)) {
            continue;
        }

        completed_lookup.insert(
                milestone);

        completed_milestones.push_back(
                milestone);

        changed_milestones.push_back(
                normalized_milestones[index]);
    }

    if (changed_milestones.is_empty()) {
        return changed_milestones;
    }

    const Error save_error =
            save_progress();

    if (save_error != OK) {
        ERR_PRINT(
                "PorchlightProgress could not save "
                "the completed milestones.");
    }

    for (int index = 0;
            index < changed_milestones.size();
            index++) {
        emit_signal(
                "milestone_completed",
                StringName(
                        changed_milestones[index]));
    }

    return changed_milestones;
}

bool PorchlightProgress::remove_milestone(
        const StringName &p_milestone) {
    PackedStringArray requested_milestones;

    requested_milestones.push_back(
            String(p_milestone));

    return !remove_milestones(
                    requested_milestones)
                    .is_empty();
}

PackedStringArray
PorchlightProgress::remove_milestones(
        const PackedStringArray &p_milestones) {
    const PackedStringArray
            normalized_milestones =
                    _normalize_milestones(
                            p_milestones);

    PackedStringArray changed_milestones;

    for (int index = 0;
            index < normalized_milestones.size();
            index++) {
        const StringName milestone =
                normalized_milestones[index];

        if (!completed_lookup.has(
                    milestone)) {
            continue;
        }

        completed_lookup.erase(
                milestone);

        completed_milestones.erase(
                milestone);

        changed_milestones.push_back(
                normalized_milestones[index]);
    }

    if (changed_milestones.is_empty()) {
        return changed_milestones;
    }

    const Error save_error =
            save_progress();

    if (save_error != OK) {
        ERR_PRINT(
                "PorchlightProgress could not save "
                "the removed milestones.");
    }

    for (int index = 0;
            index < changed_milestones.size();
            index++) {
        emit_signal(
                "milestone_removed",
                StringName(
                        changed_milestones[index]));
    }

    return changed_milestones;
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
    const String save_directory =
            save_path.get_base_dir();

    const String absolute_save_directory =
            ProjectSettings::get_singleton()
                    ->globalize_path(
                            save_directory);

    const Error directory_error =
            DirAccess::
                    make_dir_recursive_absolute(
                            absolute_save_directory);

    if (directory_error != OK) {
        return directory_error;
    }

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
            SAVE_VERSION_KEY,
            CURRENT_SAVE_FORMAT_VERSION);

    config->set_value(
            SAVE_SECTION,
            SAVE_KEY,
            saved_milestones);

    return config->save(save_path);
}

Error PorchlightProgress::load_progress() {
    return _load_progress(true);
}

Error PorchlightProgress::set_save_path(
        const String &p_save_path) {
    const String normalized_save_path =
            _normalize_save_path(
                    p_save_path);

    if (normalized_save_path.is_empty()) {
        return ERR_INVALID_PARAMETER;
    }

    if (normalized_save_path ==
            save_path) {
        return OK;
    }

    HashSet<StringName> loaded_lookup;
    Vector<StringName> loaded_milestones;

    const Error load_error =
            _read_progress(
                    normalized_save_path,
                    loaded_lookup,
                    loaded_milestones);

    if (load_error != OK) {
        return load_error;
    }

    const String previous_path =
            save_path;

    save_path =
            normalized_save_path;

    _replace_progress_state(
            loaded_lookup,
            loaded_milestones,
            true);

    emit_signal(
            "save_path_changed",
            previous_path,
            save_path);

    return OK;
}

String PorchlightProgress::
get_save_path() const {
    return save_path;
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
