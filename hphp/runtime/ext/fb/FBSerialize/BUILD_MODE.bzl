""" build mode definitions for hphp/runtime/ext/fb/FBSerialize """

# Since the `FBSerialize` rule in this directory is used by several
# components outside of fbcode, it would normally caused HHVM's
# BUILD_MODE file to get pulled in which modifies the default build
# modes for *all* projects in fbcode.  Putting this noop BUILD_MODE
# file here prevents this behavior if *only* this rule is being used
# (and HHVM builds will not be affected).

def get_modes():
    """ Return modes for this file """
    return {}
