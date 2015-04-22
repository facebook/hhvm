"""
Helpers for collecting and printing frame data.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import os
import gdb
from gdbutils import *
import idx
from nameof import nameof


#------------------------------------------------------------------------------
# Frame builders.

def create_native(idx, sp, rip, native_frame=None):
    # Try to get the function name.
    if native_frame is None:
        func_name = '<unknown>'
    else:
        try:
            func_name = native_frame.name() + '()'
        except TypeError:
            # name() returned None.
            func_name = '<unknown>'

    frame = {
        'idx':  idx,
        'sp':   str(sp),
        'rip':  _format_rip(rip),
        'func': func_name,
    }

    if native_frame is None:
        return frame

    loc = native_frame.find_sal()

    # Munge and print the code location if we have one.
    if loc is not None and loc.symtab is not None:
        filename = loc.symtab.filename

        if 'hphp' in filename:
            head, base = os.path.split(filename)
            _, basedir = os.path.split(head)
            filename = 'hphp/.../' + basedir + '/' + base

        frame['file'] = filename
        frame['line'] = loc.line

    return frame


def create_php(idx, ar, rip='0x????????', pc=None):
    """Collect metadata for a PHP frame.

    All arguments are expected to be gdb.Values, except `idx'.
    """

    func = ar['m_func']
    unit = func['m_unit']

    # Pull the function name.
    func_name = nameof(func)
    if len(func_name) == 0:
        func_name = '<pseudomain>'

    # Pull the PC from Func::base() and ar->m_soff if necessary.
    if pc is None:
        pc = rawptr(func['m_shared'])['m_base'] + ar['m_soff']

    frame = {
        'idx':  idx,
        'sp':   str(ar),
        'rip':  _format_rip(rip),
        'func': '[PHP] %s()' % func_name,
    }

    if func['m_attrs'] & V('HPHP::AttrBuiltin'):
        # Builtins don't have source files.
        return frame

    # Pull the filename from the Func or its Unit.
    filename = rawptr(rawptr(func['m_shared'])['m_originalFilename'])
    if filename == nullptr():
        filename = rawptr(unit['m_filepath'])

    frame['file'] = string_data_val(filename)

    return frame


def create_resumable(idx, resumable):
    return create_php(
        idx=idx,
        ar=resumable['m_actRec'].address,
        rip='{suspended}',
        pc=resumable['m_resumeOffset'])


#------------------------------------------------------------------------------
# Frame generators.

def gen_php(fp):
    limit = V('HPHP::s_stackLimit')
    size = V('HPHP::s_stackSize')

    while True:
        if fp - limit < size:
            break
        yield fp
        fp = fp['m_sfp']


#------------------------------------------------------------------------------
# Frame stringifiers.

def stringify(frame, sp_len=0):
    """Stringify a single frame."""

    fmt = '#{idx:<2d} {sp:<{sp_len}s} @ {rip}: {func}'
    out = fmt.format(sp_len=sp_len, **frame)

    if 'file' in frame:
        out += ' at ' + frame['file']

        if 'line' in frame:
            out += ':' + str(frame['line'])

    return out


def stringify_stacktrace(stacktrace):
    """Stringify a stacktrace, outputting a list of strings.

    Applies mild-mannered formatting magic on top of mapping stringify().
    """
    sp_len = reduce(lambda l, frm: max(l, len(frm['sp'])), stacktrace, 0)

    return [stringify(frame, sp_len) for frame in stacktrace]


#------------------------------------------------------------------------------
# Helpers.

def _format_rip(rip):
    """Hex-ify rip if it's an int-like gdb.Value."""
    try:
        rip = '0x%08x' % int(str(rip))
    except ValueError:
        rip = str(rip)

    return rip
