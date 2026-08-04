#pragma once

#include "core/io/resource.h"
#include "core/string/string_name.h"

class PorchlightAction : public Resource {
    GDCLASS(PorchlightAction, Resource);

    StringName milestone;

protected:
    static void _bind_methods();

public:
    void set_milestone(const StringName &p_milestone);
    StringName get_milestone() const;

    bool is_valid() const;
    bool execute();

    String get_description() const;
};
