"""
GDB command for printing the sizes of various containers.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# Size accessor.

def sizeof(container):
    container = deref(container)
    t = template_type(rawtype(container.type))

    if t == 'std::vector':
        impl = container['_M_impl']
        return impl['_M_finish'] - impl['_M_start']
    elif t == 'std::priority_queue':
        return sizeof(container['c'])
    elif t == 'HPHP::FixedStringMap':
        return container['m_extra']
    elif t == 'HPHP::IndexedStringMap':
        return container['m_map']['m_extra']


#------------------------------------------------------------------------------
# `sizeof' command.

class SizeOfCommand(gdb.Command):
    """Print the semantic size of a container."""

    def __init__(self):
        super(SizeOfCommand, self).__init__('sizeof', gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) != 1:
            print('Usage: sizeof <container>')
            return

        size = sizeof(argv[0])

        if size is not None:
            gdbprint(size)

SizeOfCommand()
