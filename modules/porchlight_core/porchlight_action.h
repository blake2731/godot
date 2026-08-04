#pragma once

#include "core/io/resource.h"
#include "core/string/string_name.h"

class PorchlightAction : public Resource {
    GDCLASS(PorchlightAction, Resource);

public:
    enum Operation {
        OPERATION_COMPLETE,
        OPERATION_REMOVE,
    };

private:
    StringName milestone;
    Operation operation = OPERATION_COMPLETE;

protected:
    static void _bind_methods();

public:
    void set_milestone(
            const StringName &p_milestone);

    StringName get_milestone() const;

    void set_operation(Operation p_operation);
    Operation get_operation() const;

    bool is_valid() const;
    bool execute();

    String get_description() const;
};

VARIANT_ENUM_CAST(PorchlightAction::Operation);
