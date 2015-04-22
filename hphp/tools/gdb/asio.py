"""
GDB commands for asio information and stacktraces.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import gdb
from itertools import count, izip
import re

from gdbutils import *
import frame
import idx
from nameof import nameof
from sizeof import sizeof


#------------------------------------------------------------------------------
# WaitHandle wrapper class.

class WaitHandle():
    """Wrapper class for a HHVM::c_WaitHandle*."""

    ##
    # gdb.Value delegation.
    #
    def __init__(self, wh):
        if not isinstance(wh, gdb.Value):
            raise TypeError('bad call to WaitHandle()')

        wh = deref(wh)

        if not str(wh.type).endswith('WaitHandle'):
            raise TypeError('non-WaitHandle[*] value passed to WaitHandle()')

        self.wh = wh.address
        wh_name = 'HPHP::c_' + self.kind_str() + 'WaitHandle'

        self.wh = self.wh.cast(T(wh_name).pointer())
        self.type = self.wh.type

    def __getitem__(self, idx):
        return self.wh[idx]

    def get(self):
        return self.wh

    ##
    # c_WaitHandle* methods.
    #
    def kind(self):
        kind_ty = T('HPHP::c_WaitHandle::Kind')
        return (self.wh['m_kind_state'] >> 4).cast(kind_ty)

    def kind_str(self):
        return str(self.kind())[len('HPHP::c_WaitHandle::Kind::'):]

    def state(self):
        return (self.wh['m_kind_state'] & 0xf).cast(T('uint8_t'))

    def finished(self):
        return self.state() <= K('HPHP::c_WaitHandle::STATE_FAILED')

    def resumable(self):
        resumable_ty = T('HPHP::Resumable')

        if self.type != T('HPHP::c_AsyncFunctionWaitHandle').pointer():
            return None

        p = self.wh.cast(T('char').pointer()) - resumable_ty.sizeof
        return p.cast(resumable_ty.pointer())

    ##
    # Parent chain.
    #
    def parent(self):
        """Same as wh->getParentChain().firstInContext(wh->getContextIdx())."""

        ctx_idx = self.wh['m_contextIdx']
        blockable = self.wh['m_parentChain']['m_firstParent']

        while blockable != nullptr():
            wh = WaitHandle.from_blockable(blockable)

            if not wh.finished() and wh['m_contextIdx'] == ctx_idx:
                return wh

        return None

    def chain(self):
        """Generate a WaitHandle's parent chain."""
        wh = self
        while wh is not None:
            yield wh
            wh = wh.parent()

    ##
    # Static constructors.
    #
    @staticmethod
    def from_blockable(blockable):
        """Get the containing WaitHandle of an AsioBlockable*."""

        bits = blockable['m_bits']
        kind_str = 'HPHP::AsioBlockable::Kind'

        # The remaining bits point to the next blockable in the chain.
        kind = (bits & 0x7).cast(T(kind_str))

        m = re.match(kind_str + '::(\w+)WaitHandle', str(kind))
        if m is None:
            return None

        wh_name = m.group(1)

        if wh_name == 'AsyncGenerator':
            offset = 48
        elif wh_name.startswith('Gen'):
            # GenArray, GenMap, and GenVector wait handles.
            offset = 56
        else:
            # AsyncFunction, AwaitAll, and Condition wait handles.
            offset = 40

        wh_ptype = T('HPHP::c_' + wh_name + 'WaitHandle').pointer()
        wh = (blockable.cast(T('char').pointer()) - offset).cast(wh_ptype)

        if blockable != wh['m_blockable'].address:
            return None

        return WaitHandle(wh)


#------------------------------------------------------------------------------
# Other ASIO helpers.

def asio_context(ctx_idx=None):
    """Get the AsioContext in the current thread by index."""

    contexts = V('HPHP::AsioSession::s_current')['m_p']['m_contexts']
    top_idx = sizeof(contexts)

    if ctx_idx is None:
        ctx_idx = top_idx

    if ctx_idx > top_idx:
        return None

    # AsioContexts are numbered from 1.
    return idx.vector_at(contexts, ctx_idx - 1)


#------------------------------------------------------------------------------
# ASIO stacktraces.

def asio_stacktrace(wh):
    stacktrace = []

    for wh in WaitHandle(wh).chain():
        resumable = wh.resumable()

        if resumable is not None:
            stacktrace.append(frame.create_resumable(
                len(stacktrace), resumable))

    ar = asio_context(wh['m_contextIdx'])['m_savedFP']

    if ar != nullptr():
        stacktrace.append(frame.create_php(idx=len(stacktrace), ar=ar))

    return stacktrace


class AsyncStkCommand(gdb.Command):
    """Dump the async function stacktrace for a given WaitHandle.

The format used is the same as that used by `walkstk'.
    """

    def __init__(self):
        super(AsyncStkCommand, self).__init__('asyncstk', gdb.COMMAND_STACK)

    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) != 1:
            print('Usage: asyncstk wait-handle')

        try:
            stacktrace = asio_stacktrace(argv[0])
        except TypeError:
            print('asyncstk: Argument must be a WaitHandle object or pointer.')
            return

        for frm in frame.stringify_stacktrace(stacktrace):
            print(frm)

AsyncStkCommand()
