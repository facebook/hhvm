"""
Helpers for accessing C++ STL containers in GDB.
"""
# @lint-avoid-python-3-compatibility-imports

import gdb
import re
from gdbutils import *


#------------------------------------------------------------------------------

def vector_at(vec, idx):
    return vec['_M_impl']['_M_start'][idx]


#------------------------------------------------------------------------------
# `idx' command.

class IdxCommand(gdb.Command):
    """Index into an arbitrary container.

    Usage: idx <container> <index>

If `container' is of a recognized type (e.g., native arrays, std::vector),
`idx' will index according to operator[].  Otherwise, it will attempt to treat
`container' as an object with data member `index'.
"""

    def __init__(self):
        super(IdxCommand, self).__init__('idx', gdb.COMMAND_DATA)

        # We need to convert stringified gdb.Values with struct types into a
        # form that the gdb `print` command accepts.
        self.regexes = [re.compile(pat) for pat in [
            '\s*',                  # whitespace
            '\w*=',                 # member names
            '"(?:.(?!\\\\"))*"',    # string prints
            "'(?:.(?!\\\\'))*'",    # char prints
        ]]

    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) != 2:
            print 'Usage: idx <container> <index>'
            return

        container = argv[0]
        idx = argv[1]
        value = None

        container_type = str(argv[0].type).split('<')[0]

        if container_type == 'std::vector':
            value = vector_at(container, idx)

        # Objects are just containers for data members, right?
        if value is None:
            try: value = container[idx]
            except: pass

        if value is None:
            print 'idx: Unrecognized container.'
            return

        value_str = unicode(value)
        true_type = value.type.strip_typedefs()

        if true_type.code == gdb.TYPE_CODE_STRUCT:
            for regex in self.regexes:
                value_str = regex.sub('', value_str)

        gdb.execute('print (%s)%s' % (str(value.type), value_str))

IdxCommand()
