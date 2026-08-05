#pragma once

#include "core/error/error_list.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"

class PorchlightProgress : public Object {
    GDCLASS(PorchlightProgress, Object);

    HashSet<StringName> completed_lookup;
    Vector<StringName> completed_milestones;

    PackedStringArray _normalize_milestones(
            const PackedStringArray &p_milestones) const;

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

    String get_save_path() const;

    void clear_milestones();
};
