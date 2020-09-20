#!/usr/bin/env python3

"""
Helpers for accessing HHVM's ringbuffer tracing facility.
"""

import gdb

from gdbutils import *


def rb_print(limit, filter_thread=None):
    if filter_thread is not None:
        filter_thread = filter_thread & 0xffffffff
    rbuf = V('HPHP::Trace::g_ring_ptr')
    g_idx = V('HPHP::Trace::g_ringIdx')['_M_i']
    max_idx = V('HPHP::Trace::kMaxRBEntries')

    i = 0
    while limit > 0:
        idx = (g_idx - i - 1) % max_idx
        i += 1

        entry = rbuf[idx]
        entry_type = str(entry['type']).replace('HPHP::Trace::RBType', '')
        if entry_type == 'Uninit' or idx == g_idx:
            break

        thread_id = entry['threadId']
        if filter_thread is not None and thread_id != filter_thread:
            continue
        limit -= 1

        out = '%8d %-20s ' % (entry['seq'], entry_type)
        if filter_thread is None:
            out = '0x%x ' % (thread_id) + out

        msg_fmt = '%-50s 0x%x'
        if entry_type == 'Generic':
            info = entry['generic']
            out += msg_fmt % (info['name'].string(), info['data'])
        elif entry_type == 'ServiceReq':
            info = entry['vmPoint']
            sr = str(
                info['sk'].cast(T('HPHP::jit::ServiceRequest'))
            ).replace('HPHP::jit::', '')
            out += msg_fmt % (sr, info['data'])
        elif entry_type in ['Msg', 'FuncPrologue', 'FuncEntry', 'FuncExit']:
            info = entry['msg']
            msg_len = info['len']
            msg = '' if msg_len < 1 else gdb.parse_and_eval(
                '*(const char*)%d@%d' % (info['msg'], info['len'])
            ).string()
            out += msg_fmt % (msg, info['truncatedRip'])
        else:
            info = entry['vmPoint']
            sk = info['sk'].cast(T('HPHP::SrcKey'))
            out += msg_fmt % (sk, info['data'])

        print(out)


class RbprintCommand(gdb.Command):
    """Print information from the global tracing ring buffer.

rbprint <limit> [thread]

Prints up to `limit` entries in reverse chronological order, starting with the
most recent.

`thread` may be given to limit to entries from a specific thread, and should be
the bottom >= 32 bits of the thread identifier (e.g., 0x7fce73004700 and
0x73004700 refer to the same thread).
    """
    def __init__(self):
        super(RbprintCommand, self).__init__('rbprint', gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args, 2)
        if len(argv) == 1:
            return rb_print(argv[0])

        if len(argv) == 2:
            return rb_print(argv[0], argv[1])

        print("Unexpected number of arguments: " + len(argv))


RbprintCommand()
