#pragma once

#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"

class PorchlightProgress : public Object {
    GDCLASS(PorchlightProgress, Object);

    HashSet<StringName> completed_lookup;
    Vector<StringName> completed_milestones;

protected:
    static void _bind_methods();

public:
    bool complete_milestone(const StringName &p_milestone);
    bool is_complete(const StringName &p_milestone) const;

    Array get_completed_milestones() const;
    int get_completed_count() const;

    void clear_milestones();
};
