#!/usr/bin/env python3

"""
GDB commands for inspecting APC state.
"""

from compatibility import *

import gdb

from gdbutils import *


class ApcIterator:
    """
    Iterator for contents of APC concurrent store
    """

    def __init__(self, symbol_name):
        map_type = T("HPHP::ConcurrentTableSharedStore").pointer()
        apc_ptr = TL(symbol_name).address

        self.apc_map = apc_ptr.cast(map_type)['m_vars']
        self.bucket_idx = 0
        self.bucket = self.apc_map["my_embedded_segment"]
        self.node_ptr = self.bucket[self.bucket_idx]["node_list"]
        self.index = 0
        self.segment = 0
        self.done = False

        self.key_type = self.apc_map.type.template_argument(0)
        self.value_type = self.apc_map.type.template_argument(1)

    def advance_bucket(self):
        self.index += 1
        while self.index <= int(self.apc_map["my_mask"].cast(T("size_t"))):
            if self.index & (self.index - 2) != 0:
                self.bucket_idx += 1
            else:
                self.segment += 1
                base = (1 << self.segment) & ~1
                h = self.index - base
                self.bucket_idx = h
                self.bucket = self.apc_map['my_table'][self.segment]

            self.node_ptr = self.bucket[self.bucket_idx]["node_list"]

            if int(self.node_ptr.cast(T("uintptr_t"))) > 63:
                return
            else:
                self.index += 1

        self.done = True

    def __next__(self):
        if self.done:
            raise StopIteration

        try:
            key = self.node_key().dereference()
        except gdb.MemoryError:
            key = '<invalid>'

        try:
            val = self.node_value()
        except gdb.MemoryError:
            val = '<invalid>'

        self.node_ptr = self.node_ptr["next"]
        if self.node_ptr == nullptr():
            self.advance_bucket()

        return (key, val)

    def __iter__(self):
        return self

    def node_key(self):
        return (self.node_ptr + 1).cast(self.key_type.pointer())

    def node_value(self):
        # TODO(mcolavita): Alignment concerns
        return (self.node_key() + 1).cast(self.value_type.pointer())


class DumpApcCommand(gdb.Command):
    """Dump the contents of APC in its entirety"""

    def __init__(self):
        super(DumpApcCommand, self).__init__('info apc', gdb.COMMAND_STATUS)

    @errorwrap
    def invoke(self, args, from_tty):
        apc_iter = ApcIterator("_ZN4HPHP12_GLOBAL__N_113s_apc_storageE")
        for (key, val) in apc_iter:
            print(key)
            print(val)
            print("")


DumpApcCommand()
