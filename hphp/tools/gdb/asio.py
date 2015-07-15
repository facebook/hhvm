"""
GDB commands for asio information and stacktraces.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from itertools import count
import re

from gdbutils import *
import frame
import idx
from nameof import nameof
from sizeof import sizeof


#------------------------------------------------------------------------------
# WaitHandle wrapper class.

class WaitHandle(object):
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

    def state_str(self):
        kind = self.kind_str()
        state = self.state()
        res = 'INVALID'

        # Each WaitHandle has its own states...
        if state == 0:
            res = 'SUCCEEDED'
        elif state == 1:
            res = 'FAILED'
        elif state == 2:
            if kind == 'Sleep' or kind == 'ExternalThreadEvent':
                res = 'WAITING'
            elif kind == 'Reschedule':
                res = 'SCHEDULED'
            else:
                res = 'BLOCKED'
        elif state == 3 and kind == 'Resumable':
            res = 'SCHEDULED'
        elif state == 4 and kind == 'Resumable':
            res = 'RUNNING'

        return res

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

        # AsyncFunctionWaitHandles have a slightly different layout.
        try:
            if blockable != wh['m_blockable'].address:
                return None
        except:
            if blockable != wh['m_children'][0]['m_blockable'].address:
                return None


        return WaitHandle(wh)


#------------------------------------------------------------------------------
# Other ASIO helpers.

def asio_context(ctx_idx=None):
    """Get the AsioContext in the current thread by index."""

    contexts = TL('HPHP::AsioSession::s_current')['m_p']['m_contexts']
    top_idx = sizeof(contexts)

    if ctx_idx is None:
        ctx_idx = top_idx

    if ctx_idx > top_idx:
        return None

    # AsioContexts are numbered from 1.
    return idx.vector_at(contexts, ctx_idx - 1)


#------------------------------------------------------------------------------
# ASIO stacktraces.

def asio_stacktrace(wh, limit=None):
    """Produce a list of async frames by following a WaitHandle's parent chain.
    The stacktrace ends at the WaitHandle::join().

    The whole chain is walked even if `limit' is provided---the return
    stacktrace will have `limit' or fewer entries if there were `limit' or
    fewer frames, and `limit' + 1 frames otherwise, where the last frame is the
    WaitHandle::join().
    """
    stacktrace = []
    count = 0

    for wh in WaitHandle(wh).chain():
        resumable = wh.resumable()

        if resumable is None:
            continue

        if limit is None or count < limit:
            stacktrace.append(frame.create_resumable(count, resumable))
        count += 1

    ar = asio_context(wh['m_contextIdx'])['m_savedFP']

    if ar != nullptr():
        stacktrace.append(frame.create_php(idx=count, ar=ar))

    return stacktrace


class AsyncStkCommand(gdb.Command):
    """Dump the async function stacktrace for a given WaitHandle.

The format used is the same as that used by `walkstk'.
    """

    def __init__(self):
        super(AsyncStkCommand, self).__init__('asyncstk', gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        try:
            wh = gdb.parse_and_eval(args)
        except gdb.error:
            print('Usage: asyncstk wait-handle')
            return

        try:
            stacktrace = asio_stacktrace(wh)
        except TypeError:
            print('asyncstk: Argument must be a WaitHandle object or pointer.')
            return

        for s in frame.stringify_stacktrace(stacktrace):
            print(s)

AsyncStkCommand()


#------------------------------------------------------------------------------
# `info asio' command.

class InfoAsioCommand(gdb.Command):
    """Metadata about the currently in-scope AsioContext"""

    def __init__(self):
        super(InfoAsioCommand, self).__init__('info asio', gdb.COMMAND_STATUS)

    @errorwrap
    def invoke(self, args, from_tty):
        asio_session = TL('HPHP::AsioSession::s_current')['m_p']

        contexts = asio_session['m_contexts']
        num_contexts = sizeof(contexts)

        if num_contexts == 0:
            print('Not currently in the scope of an AsioContext')
            return

        asio_ctx = asio_context()

        # Count the number of contexts, and print the topmost.
        print('\n%d stacked AsioContext%s (current: (%s) %s)' % (
            int(num_contexts),
            plural_suffix(num_contexts),
            str(asio_ctx.type),
            str(asio_ctx)))

        # Get the current vmfp().
        header_ptype = T('HPHP::rds::Header').pointer()
        vmfp = TL('HPHP::rds::tl_base').cast(header_ptype)['vmRegs']['fp']

        wh_ptype = T('HPHP::c_WaitableWaitHandle').pointer()

        # Find the most recent join().
        for i, fp in izip(count(), frame.gen_php(vmfp)):
            if nameof(fp['m_func']) == 'HH\WaitHandle::join':
                break

        if nameof(fp['m_func']) != 'HH\WaitHandle::join':
            print("...but couldn't find join().  Something is wrong.\n")
            return

        wh = fp['m_this'].cast(wh_ptype)

        print('\nCurrently %s WaitHandle: (%s) %s [state: %s]' % (
            'joining' if i == 0 else 'executing',
            str(wh.type),
            str(wh),
            WaitHandle(wh).state_str()))

        # Dump the async stacktrace.
        for s in frame.stringify_stacktrace(asio_stacktrace(wh)):
            print('    %s' % s)

        # Count the number of queued runnables.
        queue_size = sizeof(asio_ctx['m_runnableQueue'])
        print('%d other resumable%s queued' % (
            int(queue_size),
            plural_suffix(queue_size)))

        sleeps = asio_ctx['m_sleepEvents']
        externals = asio_ctx['m_externalThreadEvents']

        num_sleeps = sizeof(sleeps)
        num_externals = sizeof(externals)

        # Count sleep and external thread events.
        print('')
        print('%d pending sleep event%s' % (
            int(num_sleeps), plural_suffix(num_sleeps)))
        print('%d pending external thread event%s' % (
            int(num_externals), plural_suffix(num_externals)))

        # Dump sleep and external thread event stacktraces.
        for vec in [sleeps, externals]:
            for i in xrange(int(sizeof(vec))):
                wh = idx.vector_at(vec, i)
                stacktrace = frame.stringify_stacktrace(asio_stacktrace(wh, 3))

                print('\n(%s) %s [state: %s]' % (
                    str(wh.type), str(wh), WaitHandle(wh).state_str()))

                if len(stacktrace) == 4:
                    for s in stacktrace[0:-1]:
                        print('    %s' % s)
                    print('     ...')
                    print('    %s' % stacktrace[-1])
                else:
                    for s in stacktrace:
                        print('    %s' % s)
        print('')

InfoAsioCommand()
