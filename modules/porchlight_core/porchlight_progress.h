#pragma once

#include "core/error/error_list.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"

class PorchlightProgress : public Object {
    GDCLASS(PorchlightProgress, Object);

    HashSet<StringName> completed_lookup;
    Vector<StringName> completed_milestones;

    String save_path;

    PackedStringArray _normalize_milestones(
            const PackedStringArray &p_milestones) const;

    String _normalize_save_path(
            const String &p_save_path) const;

    String _get_temporary_save_path(
            const String &p_primary_path) const;

    String _get_backup_save_path(
            const String &p_primary_path) const;

    Error _read_progress_file(
            const String &p_file_path,
            HashSet<StringName> &r_loaded_lookup,
            Vector<StringName> &r_loaded_milestones,
            bool p_allow_missing) const;

    Error _validate_progress_file(
            const String &p_file_path) const;

    Error _write_progress_file(
            const String &p_file_path) const;

    Error _copy_validated_progress_file(
            const String &p_source_path,
            const String &p_destination_path) const;

    Error _restore_progress_backup(
            const String &p_primary_path,
            const String &p_backup_path) const;

    Error _promote_temporary_progress(
            const String &p_temporary_path,
            const String &p_primary_path,
            const String &p_backup_path,
            bool p_has_valid_backup) const;

    Error _read_progress(
            const String &p_primary_path,
            HashSet<StringName> &r_loaded_lookup,
            Vector<StringName> &r_loaded_milestones) const;

    void _replace_progress_state(
            const HashSet<StringName> &p_loaded_lookup,
            const Vector<StringName> &p_loaded_milestones,
            bool p_emit_reload_signal);

    Error _load_progress(bool p_emit_reload_signal);

protected:
    static void _bind_methods();

public:
    PorchlightProgress();

    bool complete_milestone(
            const StringName &p_milestone);

    PackedStringArray complete_milestones(
            const PackedStringArray &p_milestones);

    bool remove_milestone(
            const StringName &p_milestone);

    PackedStringArray remove_milestones(
            const PackedStringArray &p_milestones);

    bool is_complete(
            const StringName &p_milestone) const;

    Array get_completed_milestones() const;
    int get_completed_count() const;

    Error save_progress();
    Error load_progress();

    Error set_save_path(
            const String &p_save_path);

    String get_save_path() const;

    void clear_milestones();
};
