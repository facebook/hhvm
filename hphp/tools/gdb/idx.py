"""
Helpers for accessing C++ STL containers in GDB.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
import re
from gdbutils import *
from hashes import hash_of


#------------------------------------------------------------------------------
# STL accessors.
#
# These are only designed to work for gcc-4.8.1.

def atomic_get(atomic):
    inner = rawtype(atomic.type).template_argument(0)

    if inner.code == gdb.TYPE_CODE_PTR:
        return atomic['_M_b']['_M_p']
    else:
        return atomic['_M_i']


def vector_at(vec, idx):
    vec = vec['_M_impl']

    if idx >= vec['_M_finish'] - vec['_M_start']:
        return None
    else:
        return vec['_M_start'][idx]


def unordered_map_at(umap, key):
    h = umap['_M_h']

    bucket = h['_M_buckets'][hash_of(key) % h['_M_bucket_count']]
    if bucket == 0x0:
        return None

    node = bucket['_M_nxt']

    value_type = T(str(umap.type) + '::value_type').pointer()

    while node != 0x0:
        # Hashtable nodes contain only a pointer to the next node in the
        # bucket, but are always immediately followed by the value pair.
        value = (node + 1).cast(value_type)
        if key == value['first']:
            return value['second']

        node = node['_M_nxt']

    return None


#------------------------------------------------------------------------------
# Boost accessors.

def boost_flat_map_at(flat_map, key):
    vec = flat_map['m_flat_tree']['m_data']['m_vect']['members_']

    first = vec['m_start']
    last = first + vec['m_size']
    diff = last - first

    # Find lower bound.
    while diff > 0:
        half = diff >> 1
        middle = first
        middle += half

        if middle['first'] < key:
            middle += 1
            first = middle
            diff = diff - half - 1
        else:
            diff = half

    if first != last and key < first['first']:
        first = last

    return first['second'] if first != last else None


#------------------------------------------------------------------------------
# TBB accessors.

def tbb_atomic_get(atomic):
    return atomic['rep']['value']


def tbb_chm_at(chm, key):
    h = hash_of(key) & tbb_atomic_get(chm['my_mask'])

    # This simulates an Intel BSR (i.e., lg2) on h|1.
    s = len('{0:b}'.format(int(h | 1))) - 1     # segment_idx
    h -= 1 << s & ~1                            # segment_base

    # bucket_accessor b(this, h & m) <=> this->get_bucket(h & m).
    seg = chm['my_table'][s]
    b = seg[h].address

    if b['node_list'] == V('tbb::interface5::internal::rehash_req'):
        # Rehash required, so just give up.
        return None

    node_ptype = T(str(rawtype(chm.type)) + '::node').pointer()
    node = b['node_list'].cast(node_ptype)

    # search_bucket(key, b).
    while node.cast(T('size_t')) > 63 and key != node['item']['first']:
        node = node['next'].cast(node_ptype)

    if node == nullptr():
        return None

    return node['item']['second']


#------------------------------------------------------------------------------
# HHVM accessors.

def thread_local_get(tl):
    # Only works if USE_GCC_FAST_TLS was defined; deal with it.
    return tl['m_node']['m_p']


def atomic_vector_at(av, idx):
    size = av['m_size']

    if idx < size:
        return atomic_get(rawptr(av['m_vals'])[idx])
    else:
        return atomic_vector_at(atomic_get(av['m_next']), idx - size)


def fixed_vector_at(fv, idx):
    return rawptr(fv['m_sp'])[idx]


def fixed_string_map_at(fsm, sd):
    case_sensitive = rawtype(fsm.type).template_argument(1)
    sinfo = strinfo(sd, case_sensitive)

    if sinfo is None:
        return None

    elm = fsm['m_table'][-1 - (sinfo['hash'] & fsm['m_mask'])].address

    while True:
        sd = rawptr(elm['sd'])

        if sd == 0:
            return None

        if sinfo['data'] == string_data_val(sd, case_sensitive):
            return elm['data']

        elm = elm + 1
        if elm == fsm['m_table']:
            elm = elm - (fsm['m_mask'] + 1)


def _ism_index(ism, s):
    return fixed_string_map_at(ism['m_map'], s)

def _ism_access_list(ism):
    t = rawtype(rawtype(ism.type).template_argument(0))
    return ism['m_map']['m_table'].cast(t.pointer())

def indexed_string_map_at(ism, idx):
    # If `idx' is a string, it must be converted to an index via the underlying
    # FixedStringMap.
    sinfo = strinfo(idx)
    if sinfo is not None:
        idx = _ism_index(ism, idx)

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


def object_data_at(obj, prop_name):
    cls = rawptr(obj['m_cls'])

    prop_vec = _object_data_prop_vec(obj)
    prop_ind = _ism_index(cls['m_declProperties'], prop_name)

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
        'boost::container::flat_map':
                                    boost_flat_map_at,
        'tbb::interface5::concurrent_hash_map':
                                    tbb_chm_at,
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

    @errorwrap
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

    @errorwrap
    def invoke(self, container, val):
        return idx(container, val)


IdxCommand()
IdxFunction()
