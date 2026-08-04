#pragma once

#include "core/io/resource.h"
#include "core/string/string_name.h"

class PorchlightCondition : public Resource {
    GDCLASS(PorchlightCondition, Resource);

    StringName milestone;
    bool inverted = false;

protected:
    static void _bind_methods();

public:
    void set_milestone(const StringName &p_milestone);
    StringName get_milestone() const;

    void set_inverted(bool p_inverted);
    bool is_inverted() const;

    bool is_met() const;
    String get_description() const;
};
