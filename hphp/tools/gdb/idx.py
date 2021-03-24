#!/usr/bin/env python3

"""
Helpers for accessing C++ STL containers in GDB.
"""

from compatibility import *

import gdb
import re
from gdbutils import *
from hashes import hash_of


#------------------------------------------------------------------------------
# STL accessors.

def vector_at(vec, idx, hasher=None):
    vec = vec['_M_impl']

    if idx >= vec['_M_finish'] - vec['_M_start']:
        return None
    else:
        return vec['_M_start'][idx]


def unordered_map_at(umap, key, hasher=None):
    h = umap['_M_h']

    bucket = h['_M_buckets'][hash_of(key, hasher) % h['_M_bucket_count']]
    if bucket == 0x0:
        return None

    node = bucket['_M_nxt']

    value_type = T(str(rawtype(umap.type)) + '::value_type').pointer()

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

def boost_flat_map_at(flat_map, key, hasher=None):
    vec = flat_map['m_flat_tree']['m_data']['m_vect']['m_holder']

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
    try:
        return atomic['rep']['value']
    except gdb.error:
        # atomic_impl representation since version 4.1 Update 2
        return atomic['my_storage']['my_value']
    return atomic['rep']['value']


def tbb_chm_at(chm, key, hasher=None):
    h = hash_of(key, hasher) & tbb_atomic_get(chm['my_mask'])

    # This simulates an Intel BSR (i.e., lg2) on h|1.
    s = len('{0:b}'.format(int(h | 1))) - 1     # segment_idx
    h -= 1 << s & ~1                            # segment_base

    # bucket_accessor b(this, h & m) <=> this->get_bucket(h & m).
    try:
        seg = chm['my_table'][s]
    except:
        return None
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


def atomic_vector_at(av, idx, hasher=None):
    size = av['m_size']

    if idx < size:
        return atomic_get(rawptr(av['m_vals'])[idx])
    else:
        return atomic_vector_at(atomic_get(av['m_next']), idx - size)


def atomic_low_ptr_vector_at(av, idx, hasher=None):
    size = av['m_size']

    if idx < size:
        return rawptr(rawptr(av['m_vals'])[idx])
    else:
        return atomic_low_ptr_vector_at(atomic_get(av['m_next']), idx - size)


def fixed_vector_at(fv, idx, hasher=None):
    return rawptr(fv['m_sp'])[idx]


def fixed_string_map_at(fsm, sd, hasher=None):
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


def indexed_string_map_at(ism, idx, hasher=None):
    # If `idx' is a string, it must be converted to an index via the underlying
    # FixedStringMap.
    sinfo = strinfo(idx)
    if sinfo is not None:
        idx = _ism_index(ism, idx)

    if idx is not None:
        return _ism_access_list(ism)[idx]

    return None


def tread_hash_map_at(thm, key, hasher=None):
    table = atomic_get(thm['m_table'])
    capac = table['capac']

    idx = (hash_of(key, hasher) & (capac - 1)).cast(T('size_t'))

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


def compact_vector_at(vec, idx, hasher=None):
    if vec['m_data'] == nullptr():
        return None

    sz = vec['m_data']['m_len']
    if idx >= sz:
        return None

    inner = vec.type.template_argument(0)
    elems = (vec['m_data'].cast(T('char').pointer())
             + vec['elems_offset']).cast(inner.pointer())
    return elems[idx]


#------------------------------------------------------------------------------
# PHP value accessors.

def _un_quick_index(qi):
    repr_type = qi.type.strip_typedefs()

    if repr_type == T("uint16_t"):
        return qi
    elif repr_type == T("HPHP::tv_layout::detail_7up::quick_index"):
        return qi["quot"] * 7 + qi["rem"]
    else:
        return None


def tv_layout_at(layout_type, props_base, idx):
    try:
        idx = int(idx)
        quot = idx // 7
        rem = idx % 7
        chunk = props_base + T("HPHP::Value").sizeof * 8 * quot
        ty = (chunk + rem).cast(T("HPHP::DataType").pointer()).dereference()
        valaddr = chunk + T("HPHP::Value").sizeof * (1 + rem)
        val = valaddr.cast(T("HPHP::Value").pointer()).dereference()
        return pretty_tv(ty, val)
    except:
        pass

    print("\nNo such tv layout: %s\n" % layout_type.name)


def object_data_at(obj, cls, prop_name_or_slot, hasher=None):
    sinfo = strinfo(prop_name_or_slot)
    if sinfo is None:
        slot = prop_name_or_slot
    else:
        slot = _ism_index(cls['m_declProperties'], prop_name_or_slot)

    if slot is None:
        return None

    idx = _un_quick_index(fixed_vector_at(cls['m_slotIndex']['m_impl'], slot))

    if idx is None:
        return None

    props_base = (obj.address + 1).cast(T('char').pointer())

    return tv_layout_at(T("HPHP::ObjectProps"), props_base, idx)


def packed_array_at(base, idx):
    try:
        idx = int(idx)
        base = base.cast(T('char').pointer())
        quot = idx // 8
        rem = idx % 8
        chunk = base + T("HPHP::PackedBlock").sizeof * quot
        tyaddr = chunk + rem
        ty = tyaddr.cast(T("HPHP::DataType").pointer()).dereference()
        valaddr = chunk + T("HPHP::Value").sizeof * (1 + rem)
        val = valaddr.cast(T("HPHP::Value").pointer()).dereference()
        return pretty_tv(ty, val)
    except:
        pass


#------------------------------------------------------------------------------
# `idx' command.

@memoized
def idx_accessors():
    return {
        'std::vector':              vector_at,
        'std::unordered_map':       unordered_map_at,
        'HPHP::jit::hash_map':      unordered_map_at,
        'HPHP::hphp_hash_map':      unordered_map_at,
        'boost::container::'
        'flat_map':                 boost_flat_map_at,
        'tbb::interface5::'
        'concurrent_hash_map':      tbb_chm_at,
        'HPHP::AtomicVector':       atomic_vector_at,
        'HPHP::AtomicLowPtrVector': atomic_low_ptr_vector_at,
        'HPHP::FixedVector':        fixed_vector_at,
        'HPHP::FixedStringMap':     fixed_string_map_at,
        'HPHP::IndexedStringMap':   indexed_string_map_at,
        'HPHP::TreadHashMap':       tread_hash_map_at,
        'HPHP::ObjectData':         object_data_at,
        'HPHP::CompactVector':      compact_vector_at,
    }


def idx(container, index, hasher=None):
    value = None

    if container.type.code == gdb.TYPE_CODE_REF:
        container = container.referenced_value()

    container_type = template_type(container.type)
    true_type = template_type(container.type.strip_typedefs())

    accessors = idx_accessors()

    if container_type in accessors:
        value = accessors[container_type](container, index, hasher)
    elif true_type in accessors:
        value = accessors[true_type](container, index, hasher)
    else:
        try:
            value = container[index]
        except:
            print(
                'idx: Unrecognized container (%s - %s).' % (
                    container_type, true_type
                )
            )
            return None

    return value


class IdxCommand(gdb.Command):
    """Index into an arbitrary container.

    Usage: idx <container> <key> [hasher]

GDB `print` is called on the address of the value, and then the value itself is
printed.

If `container' is of a recognized type (e.g., native arrays, std::vector),
`idx' will index according to operator[].  Otherwise, it will attempt to treat
`container' as an object with data member `key'.

If `container' is accessed by hashing `key', an optional `hasher' specification
(a bare word string, such as "id", sans quotes) may be passed.  The specified
hash, if valid, will be used instead of the default hash for the key type.
"""

    def __init__(self):
        super(IdxCommand, self).__init__('idx', gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args, 2)

        if len(argv) == 2:
            value = idx(argv[0], argv[1])
        elif len(argv) == 3:
            value = idx(argv[0], argv[1], argv[2])
        else:
            print('Usage: idx <container> <key> [hasher]')
            return

        if value is None:
            print('idx: Element not found.')
            return None

        ty = str(value.type.pointer())
        ty_parts = [
            x for x in
            re.split(r'(\s*(?:const\s*)?[*&](?!\s*[>,]))', ty, 1) if x
        ]
        ty_parts[0] = "'%s'" % ty_parts[0]

        gdb.execute('print *(%s)%s' % (
            ''.join(ty_parts), str(value.address)))


class IdxFunction(gdb.Function):
    def __init__(self):
        super(IdxFunction, self).__init__('idx')

    @errorwrap
    def invoke(self, container, val):
        return idx(container, val)


IdxCommand()
IdxFunction()
