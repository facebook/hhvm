"""
GDB command for printing the names of various objects.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# Name accessor.

def nameof(val):
    val = deref(val)
    t = val.type.name

    sd = None

    if t == 'HPHP::Func':
        sd = val['m_fullName']
    elif t == 'HPHP::Class':
        sd = deref(val['m_preClass'])['m_name']
    elif t == 'HPHP::ObjectData':
        cls = deref(val['m_cls'])
        sd = deref(cls['m_preClass'])['m_name']

    if sd is None:
        return None

    return string_data_val(deref(sd))


#------------------------------------------------------------------------------
# `nameof' command.

class NameOfCommand(gdb.Command):
    """Print the name of an HHVM object."""

    def __init__(self):
        super(NameOfCommand, self).__init__('nameof', gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        try:
            obj = gdb.parse_and_eval(args)
        except gdb.error:
            print('Usage: nameof <object>')
            return

        name = nameof(obj)

        if name is not None:
            print('"' + name + '"')

NameOfCommand()
