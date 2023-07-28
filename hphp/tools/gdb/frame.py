#!/usr/bin/env python3

"""
Helpers for collecting and printing frame data.
"""

from compatibility import *

import bisect
import gdb
import itertools
import os
import sqlite3
import struct

from gdbutils import *
from lookup import lookup_func_from_fp
import idx as idxs
import repo


#------------------------------------------------------------------------------
# Frame sniffing.

def is_jitted(fp, ip):
    # Get the value of `s_code', the global CodeCache pointer.
    s_code = K('HPHP::jit::tc::g_code')

    # Set the bounds of the TC.
    try:
        tc_base = s_code['m_base']
        tc_end = tc_base + s_code['m_codeSize']
    except:
        # We can't access `s_code' for whatever reason---maybe it's gotten
        # corrupted somehow.  Assume that the TC is above the data section,
        # but restricted to low memory.
        tc_base = s_code.address.cast(T('uintptr_t'))
        tc_end = 0x100000000

    return ip >= tc_base and ip < tc_end


#------------------------------------------------------------------------------
# PHP frame info.

def php_filename(func):
    func = func.cast(T('HPHP::Func').pointer())
    filename = rawptr(rawptr(func['m_shared'])['m_originalFilename'])

    if filename == nullptr():
        filename = rawptr(func['m_unit']['m_origFilepath'])

    return string_data_val(filename)


def php_line_number_from_repo(func, pc):
    """ Get the line number in the php file associated with the given function and pc

    Uses the repo.

    Args:
        func: gdb.Value[HPHP::Func*].
        pc: gdb.Value[HPHP::CompactTaggedPtr<T, uint8_t>::Opaque].

    Returns:
        line: Option[int].
    """
    # TODO(T125686040)
    # unit = func['m_unit']
    line_table = []

    # Find the upper bound for our PC.  Note that this relies on the Python
    # dict comparison operator comparing componentwise lexicographically based
    # on the alphabetical ordering of keys.
    key = {'m_pastOffset': int(pc), 'm_val': -1}
    i = bisect.bisect_right(line_table, key)

    if i == len(line_table):
        return None

    return line_table[i]['m_val']


def php_line_number(func, pc):
    """ Get the line number in the php file associated with the given function and pc

    Uses the shared lineMap if the pc is present.

    Args:
        func: gdb.Value[HPHP::Func*].
        pc: gdb.Value[HPHP::CompactTaggedPtr<T, uint8_t>::Opaque].

    Returns:
        line: int.
    """
    shared = rawptr(func['m_shared'])
    line_map = shared['m_lineMap']['val']

    if line_map is not None:
        i = 0
        while True:
            r = idxs.compact_vector_at(line_map, i)
            if r is None:
                break
            if r['first']['base'] <= pc and r['first']['past'] > pc:
                return r['second']
            i += 1

    return php_line_number_from_repo(func, pc)

#------------------------------------------------------------------------------
# Frame builders.


def create_native(idx, fp, rip, native_frame=None, name=None):
    """ Collect metadata for a native frame.

    Args:
        idx: int.
        fp: Union[int, gdb.Value(uintptr_t)].
        native_frame: Option[gdb.Frame].
        name: Option[str].

    Returns:
        dict with keys 'idx', 'fp', 'rip', 'func', and
        optionally 'file' and 'line'
    """
    # Try to get the function name.
    if native_frame is None:
        if name is None:
            func_name = '<unknown>'
        else:
            func_name = name
    else:
        try:
            func_name = native_frame.name() + '()'
        except TypeError:
            # name() returned None.
            func_name = '<unknown>'

    frame = {
        'idx':  idx,
        'fp':   str(fp),
        'rip':  _format_rip(rip),
        'func': func_name,
    }

    if native_frame is None:
        return frame

    loc = native_frame.find_sal()

    # Munge and print the code location if we have one.
    if loc is not None and loc.symtab is not None:
        frame['file'] = loc.symtab.filename
        frame['line'] = loc.line

    return frame


def create_php(idx, ar, rip='0x????????', pc=None):
    """Collect metadata for a PHP frame.

    Args:
        idx: int.
        ar: gdb.Value[HPHP::ActRec].
        rip: Union[str, gdb.Value[uintptr_t]].
        pc: Option[gdb.Value[HPHP::Offset]].

    Returns:
        dict with keys 'idx', 'fp', 'rip', 'func', and
        optionally 'file' and 'line'
    """
    func = lookup_func_from_fp(ar)  # gdb.Value[HPHP::Func*]
    shared = rawptr(func['m_shared'])  # gdb.Value[HPHP::SharedData]
    flags = shared['m_allFlags']  # gdb.Value[HPHP::Func::SharedData::Flags]

    # Pull the function name.
    if not flags['m_isClosureBody']:
        func_name = nameof(func)
    else:
        func_name = nameof(func['m_baseCls'].cast(T('HPHP::Class').pointer()))
        func_name = func_name[:func_name.find(';')]

    if len(func_name) == 0:
        func_name = '<pseudomain>'

    frame = {
        'idx':  idx,
        'fp':   str(ar),
        'rip':  _format_rip(rip),
        'func': '[PHP] %s()' % func_name,
    }

    attrs = atomic_get(func['m_attrs']['m_attrs'])  # HPHP::Attr

    if attrs & V('HPHP::AttrBuiltin'):
        # Builtins don't have source files.
        return frame

    # Pull the PC from Func::base() and ar->m_callOff if necessary.
    if pc is None:
        bc = rawptr(shared['m_bc'])
        pc = bc + (ar['m_callOffAndFlags'] >> 3)

    frame['file'] = php_filename(func)
    frame['line'] = php_line_number(func, pc)

    return frame


def create_resumable(idx, resumable):
    return create_php(
        idx=idx,
        ar=resumable['m_actRec'].address,
        rip='{suspended}',
        pc=resumable['m_suspendOffset'])


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

def stringify(frame, fp_len=0):
    """Stringify a single frame."""

    fmt = '#{idx:<2d} {fp:<{fp_len}s} @ {rip}: {func}'
    out = fmt.format(fp_len=fp_len, **frame)

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
    fp_len = reduce(lambda l, frm: max(l, len(frm['fp'])), stacktrace, 0)

    return [stringify(frame, fp_len) for frame in stacktrace]


#------------------------------------------------------------------------------
# Helpers.

def _format_rip(rip):
    """Hex-ify rip if it's an int-like gdb.Value."""
    try:
        rip = '0x%08x' % int(str(rip))
    except ValueError:
        rip = str(rip)

    return rip


class SymValueWrapper():

    def __init__(self, symbol, value):
        self.sym = symbol
        self.val = value

    def value(self):
        return self.val

    def symbol(self):
        return self.sym


class JittedFrameDecorator(gdb.FrameDecorator.FrameDecorator):

    def __init__(self, fobj, regs):
        super(JittedFrameDecorator, self).__init__(fobj)
        self.regs = regs
        frame = self.inferior_frame()
        self.ar = frame.read_register(self.regs['fp']).cast(T('HPHP::ActRec').pointer())
        self.args = []
        try:
            func = lookup_func_from_fp(self.ar)
            shared = rawptr(func['m_shared'])
            argptr = self.ar.cast(T('HPHP::TypedValue').pointer())
            nargs = func['m_paramCounts'] >> 1
            if nargs > 64:
                return None
            args = []
            i = 0
            while i < nargs:
                argptr -= 1
                try:
                    name = idxs.indexed_string_map_at(shared['m_localNames'], i)
                    name = strinfo(name)['data']
                except:
                    name = "arg_" + str(i)
                i += 1
                args.append(SymValueWrapper(name, argptr.dereference()))
            self.args = args
        except:
            pass

    def function(self):
        try:
            func = lookup_func_from_fp(self.ar)
            shared = rawptr(func['m_shared'])
            flags = shared['m_allFlags']

            if not flags['m_isClosureBody']:
                func_name = nameof(func)
            else:
                func_name = nameof(func['m_baseCls'].cast(T('HPHP::Class').pointer()))
                func_name = func_name[:func_name.find(';')]

            func_name = ("[PHP] " + func_name + "("
                         + ", ".join([arg.symbol() for arg in self.args])
                         + ")")
            return func_name

        except:
            return "??"

    def filename(self):
        try:
            func = lookup_func_from_fp(self.ar)
            return php_filename(func)
        except:
            return None

    def line(self):
        try:
            inner = self.inferior_frame().newer()
            inner_ar = inner.read_register(self.regs['fp']).cast(
                T('HPHP::ActRec').pointer())
            func = lookup_func_from_fp(self.ar)
            shared = rawptr(func['m_shared'])
            pc = shared['m_base'] + (inner_ar['m_callOffAndFlags'] >> 2)
            return php_line_number(func, pc)
        except:
            return None

    def frame_args(self):
        return self.args

    def frame_locals(self):
        try:
            func = lookup_func_from_fp(self.ar)
            argptr = self.ar.cast(T('HPHP::TypedValue').pointer())
            nargs = func['m_paramCounts'] >> 1
            shared = rawptr(func['m_shared'])
            num_loc = shared['m_numLocals']
            if num_loc < nargs or num_loc - nargs > 64:
                return None
            argptr -= nargs
            locals = []
            i = nargs
            while i < num_loc:
                argptr -= 1
                try:
                    name = idxs.indexed_string_map_at(shared['m_localNames'], i)
                    name = strinfo(name)['data']
                except:
                    name = "unnamed_" + str(i)
                i += 1
                locals.append(SymValueWrapper(name, argptr.dereference()))
            return locals
        except:
            return None


class JittedFrameFilter():

    def __init__(self):
        self.name = "JittedFrameFilter"
        self.priority = 100
        self.enabled = True
        self.regs = arch_regs()
        gdb.frame_filters[self.name] = self

    def filter(self, frame_iter):
        self.stackBase = int(TL('HPHP::s_stackLimit'))
        self.stackTop = self.stackBase + int(TL('HPHP::s_stackSize'))
        return (self.map_decorator(x) for x in frame_iter)

    def map_decorator(self, frame_decorator):
        frame = frame_decorator.inferior_frame()
        fp = frame.read_register(self.regs['fp'])
        ip = frame.read_register(self.regs['ip'])
        if is_jitted(fp, ip) and (fp < self.stackBase or fp >= self.stackTop):
            return JittedFrameDecorator(frame_decorator, self.regs)
        else:
            return frame_decorator
