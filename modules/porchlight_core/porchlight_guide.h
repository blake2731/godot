#pragma once

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"

class PorchlightGuide : public RefCounted {
    GDCLASS(PorchlightGuide, RefCounted);

protected:
    static void _bind_methods();

public:
    String get_engine_name() const;
    String get_welcome_message() const;
    String celebrate_progress(const String &p_milestone) const;
};
