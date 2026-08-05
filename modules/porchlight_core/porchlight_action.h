#pragma once

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"

class PorchlightAction : public Resource {
    GDCLASS(PorchlightAction, Resource);

public:
    enum Operation {
        OPERATION_COMPLETE,
        OPERATION_REMOVE,
    };

    enum TargetMode {
        TARGET_SINGLE = 0,
        TARGET_MULTIPLE = 1,
    };

private:
    StringName milestone;
    PackedStringArray milestones;

    Operation operation = OPERATION_COMPLETE;
    TargetMode target_mode = TARGET_SINGLE;

    PackedStringArray _get_normalized_milestones() const;

protected:
    static void _bind_methods();

    void _validate_property(
            PropertyInfo &p_property) const;

public:
    void set_milestone(
            const StringName &p_milestone);

    StringName get_milestone() const;

    void set_milestones(
            const PackedStringArray &p_milestones);

    PackedStringArray get_milestones() const;

    void set_operation(Operation p_operation);
    Operation get_operation() const;

    void set_target_mode(TargetMode p_target_mode);
    TargetMode get_target_mode() const;

    bool is_valid() const;
    bool execute();

    String get_description() const;
};

VARIANT_ENUM_CAST(PorchlightAction::Operation);
VARIANT_ENUM_CAST(PorchlightAction::TargetMode);
