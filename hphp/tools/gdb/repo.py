"""
Wow okay, let's frob repos from inside gdb.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
import sqlite3
from gdbutils import *


#------------------------------------------------------------------------------

_paths = [ None, None ]
_conns = [ None, None ]


def get(repo_id):
    global _conns
    try:
        return _conns[int(repo_id)]
    except:
        return None


def set(repo_id, path):
    global _paths, _conns

    repo_id = int(repo_id)
    if repo_id > 1:
        return

    if _conns[repo_id] is not None:
        _conns[repo_id].close()

    _paths[repo_id] = path
    _conns[repo_id] = sqlite3.connect(path)


def table(prefix):
    return '%s_%s' % (prefix, K('HPHP::kRepoSchemaId').string())


#------------------------------------------------------------------------------
# Decoder.

class Decoder(object):
    def __init__(self, buf):
        self.buf = buf
        self.off = 0
        self.struct = struct.Struct('b')

    def next_byte(self):
        return self.struct.unpack_from(self.buf[self.off:])[0]

    def decode(self):
        remaining = len(self.buf) - self.off
        max_len = 10    # int(V('folly::kMaxVarintLength64'))

        limit = min(remaining, max_len)
        i, val, shift = 0, 0, 0
        b = self.next_byte()

        while i < limit and b < 0:
            val |= (b & 0x7f) << shift
            self.off += 1
            shift += 7
            b = self.next_byte()

        if i == limit:
            raise BufferError('Invalid varint value.')

        val |= (b & 0x7f) << shift
        self.off += 1

        return val

    def finished(self):
        return self.off == len(self.buf)


#------------------------------------------------------------------------------
# `repo' commands.

class RepoCommand(gdb.Command):
    """Point GDB to HHVM's repos to fetch auxiliary data."""

    def __init__(self):
        super(RepoCommand, self).__init__('repo', gdb.COMMAND_FILES,
                                          gdb.COMPLETE_NONE, True)


class RepoShowCommand(gdb.Command):
    """List the user-specified repos."""

    def __init__(self):
        super(RepoShowCommand, self).__init__('repo show', gdb.COMMAND_FILES)

    @errorwrap
    def invoke(self, args, from_tty):
        global _paths
        print('Central repo: %s' % '<none>' if _paths[0] is None else _paths[0])
        print('Local repo: %s'   % '<none>' if _paths[1] is None else _paths[1])


class RepoSetCentralCommand(gdb.Command):
    """Set the central repo."""

    def __init__(self):
        super(RepoSetCentralCommand, self).__init__('repo set-central',
                                                    gdb.COMMAND_FILES)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)
        if len(argv) != 1:
            print('Usage: repo set-central path/to/repo')
            return

        set(V('HPHP::RepoIdCentral'), argv[0])
        print('Set central repo to %s' % argv[0])


class RepoSetLocalCommand(gdb.Command):
    """Set the local repo."""

    def __init__(self):
        super(RepoSetLocalCommand, self).__init__('repo set-local',
                                                  gdb.COMMAND_FILES)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)
        if len(argv) != 1:
            print('Usage: repo set-local path/to/repo')
            return

        set(V('HPHP::RepoIdLocal'), argv[0])
        print('Set local repo to %s' % argv[0])


RepoCommand()
RepoShowCommand()
RepoSetCentralCommand()
RepoSetLocalCommand()
