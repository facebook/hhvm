"""
Helpers for collecting and printing frame data.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import os
import gdb
import bisect
import sqlite3
import struct

from gdbutils import *
import idx as idxs
from nameof import nameof
import repo


#------------------------------------------------------------------------------
# PHP frame info.

def php_filename(func):
    filename = rawptr(rawptr(func['m_shared'])['m_originalFilename'])

    if filename == nullptr():
        filename = rawptr(func['m_unit']['m_filepath'])

    return string_data_val(filename)


def php_line_number_from_repo(func, pc):
    unit = func['m_unit']
    repo_id = unit['m_repoId']
    sn = int(unit['m_sn'])

    conn = repo.get(repo_id)
    if conn is None:
        return None

    # Query for the line table.
    c = conn.cursor()
    table = repo.table('UnitLineTable')
    c.execute('SELECT data FROM %s WHERE unitSn == ?;' % table, (sn,))

    row = c.fetchone()
    if row is None:
        return None
    buf = row[0]

    # Unpack the line table structure.
    line_table = []

    decoder = repo.Decoder(buf)
    size = decoder.decode()

    for i in xrange(size):
        line_table.append({
            'm_pastOffset': decoder.decode(),
            'm_val':        decoder.decode(),
        })

    if not decoder.finished():
        # Something went wrong.
        return None

    # Find the upper bound for our PC.  Note that this relies on the Python
    # dict comparison operator comparing componentwise lexicographically based
    # on the alphabetical ordering of keys.
    key = {'m_pastOffset': int(pc), 'm_val': -1}
    i = bisect.bisect_right(line_table, key)

    if i == len(line_table):
        return None

    return line_table[i]['m_val']


def php_line_number(func, pc):
    unit = func['m_unit']

    line_info = V('HPHP::(anonymous namespace)::s_lineInfo')
    line_map = idxs.tbb_chm_at(line_info, unit)

    if line_map is not None:
        line = idxs.boost_flat_map_at(line_map, pc)
        if line is not None:
            return line

    return php_line_number_from_repo(func, pc)


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
    shared = rawptr(func['m_shared'])

    # Pull the function name.
    if not shared['m_isClosureBody']:
        func_name = nameof(func)
    else:
        func_name = nameof(func['m_baseCls'])
        func_name = func_name[:func_name.find(';')]

    if len(func_name) == 0:
        func_name = '<pseudomain>'

    frame = {
        'idx':  idx,
        'sp':   str(ar),
        'rip':  _format_rip(rip),
        'func': '[PHP] %s()' % func_name,
    }

    attrs = idxs.atomic_get(func['m_attrs']['m_attrs'])

    if attrs & V('HPHP::AttrBuiltin'):
        # Builtins don't have source files.
        return frame

    # Pull the PC from Func::base() and ar->m_soff if necessary.
    if pc is None:
        pc = shared['m_base'] + ar['m_soff']

    # Adjust it for calls.
    op_ptype = T('HPHP::Op').pointer()
    op = (func['m_unit']['m_bc'] + pc).cast(op_ptype).dereference()

    if op in [V('HPHP::Op::' + x) for x in
              ['PopR', 'UnboxR', 'UnboxRNop']]:
        pc -= 1

    frame['file'] = php_filename(func)
    frame['line'] = php_line_number(func, pc)

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
    limit = TL('HPHP::s_stackLimit')
    size = TL('HPHP::s_stackSize')

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

    filename = frame.get('file')
    line = frame.get('line')

    if filename is not None:
        out += ' at ' + filename
        if line is not None:
            out += ':' + str(line)

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
