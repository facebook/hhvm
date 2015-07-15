"""
GDB commands for various HHVM ID lookups.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
import idx
import unit
from gdbutils import *


#------------------------------------------------------------------------------
# `lookup' command.

class LookupCommand(gdb.Command):
    """Lookup HHVM runtime objects by ID."""

    def __init__(self):
        super(LookupCommand, self).__init__('lookup', gdb.COMMAND_DATA,
                                            gdb.COMPLETE_NONE, True)

LookupCommand()


#------------------------------------------------------------------------------
# `lookup func' command.

def lookup_func(val):
    funcid = val.cast(T('HPHP::FuncId'))
    return idx.atomic_vector_at(V('HPHP::s_funcVec'), funcid)


class LookupFuncCommand(gdb.Command):
    """Lookup a Func* by its FuncId."""

    def __init__(self):
        super(LookupFuncCommand, self).__init__('lookup func',
                                                gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) != 1:
            print('Usage: lookup func <FuncId>')
            return

        gdbprint(lookup_func(argv[0]))


class LookupFuncFunction(gdb.Function):
    def __init__(self):
        super(LookupFuncFunction, self).__init__('lookup_func')

    @errorwrap
    def invoke(self, val):
        return lookup_func(val)


LookupFuncCommand()
LookupFuncFunction()


#------------------------------------------------------------------------------
# `lookup litstr' command.

def lookup_litstr(litstr_id, u):
    table = None
    gloff = V('HPHP::kGlobalLitstrOffset')

    if litstr_id >= gloff:
        litstr_id -= gloff
        u = V('HPHP::LitstrTable::s_litstrTable')

    return idx.vector_at(u['m_namedInfo'], litstr_id)['first']


class LookupLitstrCommand(gdb.Command):
    """Lookup a litstr StringData* by its Id and Unit*.

If no Unit is given, the current unit (set by `unit') is used.
"""

    def __init__(self):
        super(LookupLitstrCommand, self).__init__('lookup litstr',
                                                  gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) == 0 or len(argv) > 2:
            print('Usage: lookup litstr <Id> [Unit*]')
            return

        if len(argv) == 1:
            if unit.curunit is None:
                print('lookup litstr: No Unit set or provided.')
            u = curunit

        u = argv[0].cast(T('HPHP::Unit').pointer())
        litstr_id = argv[1].cast(T('HPHP::Id'))

        litstr = lookup_litstr(litstr_id, u)
        gdbprint(litstr)

LookupLitstrCommand()
