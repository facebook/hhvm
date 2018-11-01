""" build mode definitions for hphp/runtime/ext/fb/FBSerialize """

load("@fbcode_macros//build_defs:create_build_mode.bzl", "create_build_mode")

# Since the `FBSerialize` rule in this directory is used by several
# components outside of fbcode, it would normally caused HHVM's
# BUILD_MODE file to get pulled in which modifies the default build
# modes for *all* projects in fbcode.  Putting this BUILD_MODE
# file here prevents this behavior if *only* this rule is being used
# (and HHVM builds will not be affected).

_mode = create_build_mode(compiler = "gcc")  # T33723700

def get_modes():
    """ Return modes for this file """
    return {
        "dbg": _mode,
        "dbgo": _mode,
        "opt": _mode,
    }
