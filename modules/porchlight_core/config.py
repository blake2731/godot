def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "PorchlightAction",
        "PorchlightCondition",
        "PorchlightGuide",
        "PorchlightProgress",
        "PorchlightRule",
        "PorchlightRuleManager",
        "PorchlightRuleRunner",
    ]


def get_doc_path():
    return "doc_classes"
