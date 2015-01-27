"""
Helpers for accessing C++ STL containers in GDB.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

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


def unordered_map_at(umap, idx):
    h = umap['_M_h']

    bucket = h['_M_buckets'][hash_of(idx) % h['_M_bucket_count']]
    if bucket == 0x0:
        return None

    node = bucket['_M_nxt']

    value_type = T(str(umap.type) + '::value_type').pointer()

    while node != 0x0:
        # Hashtable nodes contain only a pointer to the next node in the
        # bucket, but are always immediately followed by the value pair.
        value = (node + 1).cast(value_type)
        if idx == value['first']:
            return value['second']

        node = node['_M_nxt']

    return None


#------------------------------------------------------------------------------
# HHVM accessors.

def atomic_vector_at(av, idx):
    return atomic_get(rawptr(av['m_vals'])[idx])


def fixed_vector_at(fv, idx):
    return rawptr(fv['m_sp'])[idx]


def fixed_string_map_at(fsm, sd):
    sd = deref(sd)

    # Give up if the StringData was never hashed.
    if int(sd['m_hash']) == 0:
        return None

    case_sensitive = rawtype(fsm.type).template_argument(1)
    s = string_data_val(sd, case_sensitive)

    elm = fsm['m_table'][-1 - (sd['m_hash'] & fsm['m_mask'])].address

    while True:
        sd = rawptr(elm['sd'])

        if sd == 0:
            return None

        if s == string_data_val(sd, case_sensitive):
            return elm['data']

        elm = elm + 1
        if elm == fsm['m_table']:
            elm = elm - (fsm['m_mask'] + 1)


def _ism_index(ism, sd):
    return fixed_string_map_at(ism['m_map'], sd)

def _ism_access_list(ism):
    t = rawtype(rawtype(ism.type).template_argument(0))
    return ism['m_map']['m_table'].cast(t.pointer())

def indexed_string_map_at(ism, idx):
    # If `idx' is a string, it must be converted to an index via the underlying
    # FixedStringMap.
    sd = rawptr(idx)
    if sd is not None:
        idx = _ism_index(ism, sd)

    if idx is not None:
        return _ism_access_list(ism)[idx]

    return None


def tread_hash_map_at(thm, key):
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
# PHP value accessors.

def _object_data_prop_vec(obj):
    cls = rawptr(obj['m_cls'])
    extra = rawptr(cls['m_extra'])

    prop_vec = (obj.address + 1).cast(T('uintptr_t')) + \
                extra['m_builtinODTailSize']
    return prop_vec.cast(T('HPHP::TypedValue').pointer())


def object_data_at(obj, sd):
    cls = rawptr(obj['m_cls'])

    prop_vec = _object_data_prop_vec(obj)
    prop_ind = _ism_index(cls['m_declProperties'], sd)

    if prop_ind is None:
        return None

    return prop_vec[prop_ind]


#------------------------------------------------------------------------------
# `idx' command.

@memoized
def idx_accessors():
    return {
        'std::vector':              vector_at,
        'std::unordered_map':       unordered_map_at,
        'HPHP::AtomicVector':       atomic_vector_at,
        'HPHP::FixedVector':        fixed_vector_at,
        'HPHP::FixedStringMap':     fixed_string_map_at,
        'HPHP::IndexedStringMap':   indexed_string_map_at,
        'HPHP::TreadHashMap':       tread_hash_map_at,
        'HPHP::ObjectData':         object_data_at,
    }


def idx(container, index):
    value = None

    if container.type.code == gdb.TYPE_CODE_REF:
        container = container.referenced_value()

    container_type = template_type(container.type)
    true_type = template_type(container.type.strip_typedefs())

    accessors = idx_accessors()

    if container_type in accessors:
        value = accessors[container_type](container, index)
    elif true_type in accessors:
        value = accessors[true_type](container, index)
    else:
        try:
            value = container[index]
        except:
            print('idx: Unrecognized container.')
            return None

    return value


class IdxCommand(gdb.Command):
    """Index into an arbitrary container.

    Usage: idx <container> <index>

GDB `print` is called on the address of the value, and then the value itself is
printed.

If `container' is of a recognized type (e.g., native arrays, std::vector),
`idx' will index according to operator[].  Otherwise, it will attempt to treat
`container' as an object with data member `index'.
"""

    def __init__(self):
        super(IdxCommand, self).__init__('idx', gdb.COMMAND_DATA)

    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) != 2:
            print('Usage: idx <container> <index>')
            return

        value = idx(argv[0], argv[1])

        if value is None:
            print('idx: Element not found.')
            return None

        gdb.execute('print *(%s)%s' % (
            str(value.type.pointer()), str(value.address)))


class IdxFunction(gdb.Function):
    def __init__(self):
        super(IdxFunction, self).__init__('idx')

    def invoke(self, container, val):
        return idx(container, val)


IdxCommand()
IdxFunction()
