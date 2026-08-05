#pragma once

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"

class PorchlightCondition : public Resource {
    GDCLASS(PorchlightCondition, Resource);

public:
    enum MatchMode {
        MATCH_SINGLE = 0,
        MATCH_ALL = 1,
        MATCH_ANY = 2,
    };

private:
    StringName milestone;
    PackedStringArray milestones;

    MatchMode match_mode = MATCH_SINGLE;
    bool inverted = false;

    PackedStringArray _get_normalized_milestones() const;

protected:
    static void _bind_methods();

public:
    void set_milestone(const StringName &p_milestone);
    StringName get_milestone() const;

    void set_milestones(
            const PackedStringArray &p_milestones);

    PackedStringArray get_milestones() const;

    void set_match_mode(MatchMode p_match_mode);
    MatchMode get_match_mode() const;

    void set_inverted(bool p_inverted);
    bool is_inverted() const;

    bool is_valid() const;

    bool references_milestone(
            const StringName &p_milestone) const;

    bool is_met() const;
    String get_description() const;
};

VARIANT_ENUM_CAST(PorchlightCondition::MatchMode);
