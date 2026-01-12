# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

import errno
import os
import select
import socket
import sys
import time
import warnings

from thrift.transport.TTransport import (
    TServerTransportBase,
    TTransportBase,
    TTransportException,
)

try:
    import fcntl
except ImportError:
    # Windows doesn't have this module
    fcntl = None


def py2_compatible_process_time():
    if sys.version_info.major >= 3 and sys.version_info.minor >= 3:
        return time.process_time()
    else:
        return time.clock()


class ConnectionEpoll:
    """epoll is preferred over select due to its efficiency and ability to
    handle more than 1024 simultaneous connections"""

    def __init__(self):
        self.epoll = select.epoll()
        # TODO should we set any other masks?
        # http://docs.python.org/library/select.html#epoll-objects
        self.READ_MASK = select.EPOLLIN | select.EPOLLPRI
        self.WRITE_MASK = select.EPOLLOUT
        self.ERR_MASK = select.EPOLLERR | select.EPOLLHUP

    def read(self, fileno):
        self.unregister(fileno)
        self.epoll.register(fileno, self.READ_MASK | self.ERR_MASK)

    def write(self, fileno):
        self.unregister(fileno)
        self.epoll.register(fileno, self.WRITE_MASK)

    def unregister(self, fileno):
        try:
            self.epoll.unregister(fileno)
        except Exception:
            pass

    def process(self, timeout):
        # poll() invokes a "long" syscall that will be interrupted by any signal
        # that comes in, causing an EINTR error.  If this happens, avoid dying
        # horribly by trying again with the appropriately shortened timout.
        process_time = py2_compatible_process_time()

        deadline = process_time + float(timeout or 0)
        poll_timeout = float(timeout or -1)
        while True:
            if timeout is not None and timeout > 0:
                poll_timeout = max(0, deadline - py2_compatible_process_time())
            try:
                msgs = self.epoll.poll(timeout=poll_timeout)
                break
            except IOError as e:
                if e.errno == errno.EINTR:
                    continue
                else:
                    raise

        rset = []
        wset = []
        xset = []
        for fd, mask in msgs:
            if mask & self.READ_MASK:
                rset.append(fd)
            if mask & self.WRITE_MASK:
                wset.append(fd)
            if mask & self.ERR_MASK:
                xset.append(fd)

        return rset, wset, xset


class ConnectionSelect:
    def __init__(self):
        self.readable = set()
        self.writable = set()

    def read(self, fileno):
        if fileno in self.writable:
            self.writable.remove(fileno)
        self.readable.add(fileno)

    def write(self, fileno):
        if fileno in self.readable:
            self.readable.remove(fileno)
        self.writable.add(fileno)

    def unregister(self, fileno):
        if fileno in self.readable:
            self.readable.remove(fileno)
        elif fileno in self.writable:
            self.writable.remove(fileno)

    def registered(self, fileno):
        return fileno in self.readable or fileno in self.writable

    def process(self, timeout):
        # select() invokes a "long" syscall that will be interrupted by any
        # signal that comes in, causing an EINTR error.  If this happens,
        # avoid dying horribly by trying again with the appropriately
        # shortened timout.
        deadline = py2_compatible_process_time() + float(timeout or 0)
        poll_timeout = timeout if timeout is None or timeout > 0 else None
        while True:
            if timeout is not None and timeout > 0:
                poll_timeout = max(0, deadline - py2_compatible_process_time())
            try:
                return select.select(
                    list(self.readable),
                    list(self.writable),
                    list(self.readable),
                    poll_timeout,
                )
            except IOError as e:
                if e.errno == errno.EINTR:
                    continue
                else:
                    raise


class TSocketBase(TTransportBase):
    """Base class for both connected and listening sockets"""

    def __init__(self):
        self.handles = {}

    def _resolveAddr(self, family=None):
        if family is None:
            family = socket.AF_UNSPEC
        if self._unix_socket is not None:
            return [(socket.AF_UNIX, socket.SOCK_STREAM, None, None, self._unix_socket)]
        else:
            ai_flags = 0
            if self.host is None:
                ai_flags |= socket.AI_PASSIVE
            return socket.getaddrinfo(
                self.host, self.port, family, socket.SOCK_STREAM, 0, ai_flags
            )

    def close(self):
        klist = (
            self.handles.keys()
            if sys.version_info[0] < 3
            else list(self.handles.keys())
        )
        for key in klist:
            self.handles[key].close()
            del self.handles[key]

    def getSocketName(self):
        if not self.handles:
            raise TTransportException(
                TTransportException.NOT_OPEN, "Transport not open"
            )
        return next(iter(self.handles.values())).getsockname()

    def fileno(self):
        if not self.handles:
            raise TTransportException(
                TTransportException.NOT_OPEN, "Transport not open"
            )
        if sys.version_info[0] >= 3:
            return list(self.handles.values())[0].fileno()
        else:
            return self.handles.values()[0].fileno()

    def setCloseOnExec(self, closeOnExec):
        self.close_on_exec = closeOnExec
        for handle in self.handles.values():
            self._setHandleCloseOnExec(handle)

    def _setHandleCloseOnExec(self, handle):
        # Windows doesn't have this module, don't set the handle in this case.
        if fcntl is None:
            return

        # Skip if fcntl.fcntl is not available (this can happen in env like Pyodide)
        if not hasattr(fcntl, "fcntl"):
            return

        flags = fcntl.fcntl(handle, fcntl.F_GETFD, 0)
        if flags < 0:
            raise IOError("Error in retrieving file options")
        if self.close_on_exec:
            fcntl.fcntl(handle, fcntl.F_SETFD, flags | fcntl.FD_CLOEXEC)
        else:
            fcntl.fcntl(handle, fcntl.F_SETFD, flags & ~fcntl.FD_CLOEXEC)


class TSocket(TSocketBase):
    """Connection Socket implementation of TTransport base."""

    def __init__(self, host="localhost", port=9090, unix_socket=None, family=None):
        """Initialize a TSocket

        @param host(str)  The host to connect to.
        @param port(int)  The (TCP) port to connect to.
        @param unix_socket(str)  The filename of a unix socket to connect to.
                                 (host and port will be ignored.)
        @param family(int)  Address family for connection. Ignored if
                            unix_socket is specified.
        """
        TSocketBase.__init__(self)
        self.host = host
        self.port = port
        self.handle = None
        self.family = family
        self._unix_socket = unix_socket
        self._timeout = None
        self.close_on_exec = True
        if not unix_socket:
            self.port = int(self.port)

    def __enter__(self):
        if not self.isOpen():
            self.open()
        return self

    def __exit__(self, type, value, traceback):
        if self.isOpen():
            self.close()

    def setHandle(self, h):
        self.handle = h
        self.handles[h.fileno()] = h

    def getHandle(self):
        return self.handle

    def close(self):
        TSocketBase.close(self)
        self.handle = None

    def isOpen(self):
        return self.handle is not None

    def setTimeout(self, ms):
        if ms is None:
            self._timeout = None
        else:
            self._timeout = ms / 1000.0

        if self.handle is not None:
            self.handle.settimeout(self._timeout)

    def getPeerName(self):
        if not self.handle:
            raise TTransportException(
                TTransportException.NOT_OPEN, "Transport not open"
            )
        return self.handle.getpeername()

    def open(self):
        address = None
        try:
            res0 = self._resolveAddr(self.family)
            for res in res0:
                address = res[4]
                handle = socket.socket(res[0], res[1])
                self.setHandle(handle)
                handle.settimeout(self._timeout)
                self.setCloseOnExec(self.close_on_exec)
                try:
                    handle.connect(address)
                except socket.error:
                    self.close()
                    if res is not res0[-1]:
                        continue
                    else:
                        raise
                break
        except socket.error as e:
            if self._unix_socket:
                msg = "socket error connecting to path %s: %s" % (
                    self._unix_socket,
                    repr(e),
                )
            else:
                msg = "socket error connecting to host %s, port %s (%s): %s" % (
                    self.host,
                    self.port,
                    repr(address),
                    repr(e),
                )
            raise TTransportException(TTransportException.NOT_OPEN, msg)

    def read(self, sz):
        try:
            buff = self.handle.recv(sz)
            if len(buff) == 0:
                raise TTransportException(
                    type=TTransportException.END_OF_FILE, message="TSocket read 0 bytes"
                )
        except socket.error as e:
            raise TTransportException(
                type=TTransportException.END_OF_FILE,
                message="Socket read failed: {}".format(str(e)),
            )
        return buff

    def write(self, buff):
        if not self.handle:
            raise TTransportException(
                TTransportException.NOT_OPEN, "Transport not open"
            )
        sent = 0
        have = len(buff)
        while sent < have:
            try:
                plus = self.handle.send(buff)
            except socket.error as e:
                raise TTransportException(
                    type=TTransportException.END_OF_FILE,
                    message="Socket write failed: {}".format(str(e)),
                )
            assert plus > 0
            sent += plus
            buff = buff[plus:]

    def flush(self):
        pass


class TServerSocket(TSocketBase, TServerTransportBase):
    """Socket implementation of TServerTransport base."""

    def __init__(self, port=9090, unix_socket=None, family=None, backlog=128):
        """Initialize a TServerSocket

        @param family(int): address family for connections. Ignored if
                            unix_socket is specified.
        @param host(str)  The host to connect to.
        @param port(int)  The (TCP) port to connect to.
        @param unix_socket(str)  The filename of a unix socket to connect to.
                                 (host, port, and family will be ignored.)
        @param backlog(int): maximum number of connections in listen queue.
        """
        TSocketBase.__init__(self)
        self.host = None
        self.port = port
        self._unix_socket = unix_socket
        self.family = family
        self.tcp_backlog = backlog
        self.close_on_exec = True
        if not unix_socket:
            self.port = int(self.port)

        # Since we now rely on select() by default to do accepts across
        # multiple socket fds, we can receive two connections concurrently.
        # In order to maintain compatibility with the existing .accept() API,
        # we need to keep track of the accept backlog.
        self._queue = []

    def __enter__(self):
        if not self.isListening():
            self.listen()
        return self

    def __exit__(self, type, value, traceback):
        if self.isListening():
            self.close()

    def getSocketName(self):
        warnings.warn(
            "getSocketName() is deprecated for TServerSocket.  "
            "Please use getSocketNames() instead."
        )
        return TSocketBase.getSocketName(self)

    def getSocketNames(self):
        return [handle.getsockname() for handle in self.handles.values()]

    def fileno(self):
        warnings.warn(
            "fileno() is deprecated for TServerSocket.  Please use filenos() instead."
        )
        return TSocketBase.fileno(self)

    def filenos(self):
        return [handle.fileno() for handle in self.handles.values()]

    def _cleanup_unix_socket(self, addrinfo):
        tmp = socket.socket(addrinfo[0], addrinfo[1])
        try:
            tmp.connect(addrinfo[4])
        except socket.error as err:
            eno, message = err.args
            if eno == errno.ECONNREFUSED:
                os.unlink(addrinfo[4])

    def isListening(self):
        return bool(self.handles)

    def listen(self):
        res0 = self._resolveAddr(self.family)

        for res in res0:
            if res[0] == socket.AF_INET6 and res[4][0] == socket.AF_INET6:
                # This happens if your version of python was built without IPv6
                # support.  getaddrinfo() will return IPv6 addresses, but the
                # contents of the address field are bogus.
                # (For example, see http://bugs.python.org/issue8858)
                #
                # Ignore IPv6 addresses if python doesn't have IPv6 support.
                continue

            # We need remove the old unix socket if the file exists and
            # nobody is listening on it.
            if self._unix_socket:
                self._cleanup_unix_socket(res)

            # Don't complain if we can't create a socket
            # since this is handled below.
            try:
                handle = socket.socket(res[0], res[1])
            except Exception:
                continue
            handle.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._setHandleCloseOnExec(handle)

            # Always set IPV6_V6ONLY for IPv6 sockets when not on Windows
            if res[0] == socket.AF_INET6 and sys.platform != "win32":
                handle.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, True)

            handle.settimeout(None)
            handle.bind(res[4])
            handle.listen(self.tcp_backlog)

            self.handles[handle.fileno()] = handle

        if not self.handles:
            raise TTransportException("No valid interfaces to listen on!")

    def _sock_accept(self):
        if self._queue:
            return self._queue.pop()

        if hasattr(select, "epoll"):
            poller = ConnectionEpoll()
        else:
            poller = ConnectionSelect()

        for filenos in self.handles.keys():
            poller.read(filenos)

        r, _, x = poller.process(0)

        for fd in r:
            self._queue.append(self.handles[fd].accept())

        if not self._queue:
            raise TTransportException("Accept interrupt without client?")

        return self._queue.pop()

    def accept(self):
        return self._makeTSocketFromAccepted(self._sock_accept())

    def _makeTSocketFromAccepted(self, accepted):
        client, addr = accepted
        result = TSocket()
        result.setHandle(client)
        return result
