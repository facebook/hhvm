"""
Helpers for accessing C++ STL containers in GDB.
"""
# @lint-avoid-python-3-compatibility-imports

import gdb
import re
from gdbutils import *
from hashes import hash_of


#------------------------------------------------------------------------------
# STL accessors.
#
# These are only designed to work for gcc-4.8.1.

def atomic_get(atomic):
    return atomic['_M_b']['_M_p']


def vector_at(vec, idx):
    vec = vec['_M_impl']

    if idx >= vec['_M_finish'] - vec['_M_start']:
        return None
    else:
        return vec['_M_start'][idx]


#------------------------------------------------------------------------------
# HHVM accessors.

def thm_at(thm, key):
    table = atomic_get(thm['m_table'])
    capac = table['capac']

    idx = (hash_of(key) & (capac - 1)).cast(T('size_t'))

    while True:
        entry = table['entries'][idx]
        probe = atomic_get(entry['first'])

        if probe == key:
            return entry['second']
        if probe == 0:
            return None

        idx += 1
        if idx == capac:
            idx = 0


#------------------------------------------------------------------------------
# Helpers.

def template_type(t):
    return str(t).split('<')[0]


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

        self.accessors = {
            'std::vector':          vector_at,
            'HPHP::TreadHashMap':   thm_at,
        }

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

        container_type = template_type(argv[0].type)
        true_type = template_type(argv[0].type.strip_typedefs())

        if container_type in self.accessors:
            value = self.accessors[container_type](container, idx)
        elif true_type in self.accessors:
            value = self.accessors[true_type](container, idx)
        else:
            try:
                value = container[idx]
            except:
                print 'idx: Unrecognized container.'
                return

        if value is None:
            print 'idx: Element not found.'
            return

        value_str = unicode(value)
        true_type = value.type.strip_typedefs()

        if true_type.code == gdb.TYPE_CODE_STRUCT:
            for regex in self.regexes:
                value_str = regex.sub('', value_str)

        gdb.execute('print (%s)%s' % (str(value.type), value_str))

IdxCommand()
