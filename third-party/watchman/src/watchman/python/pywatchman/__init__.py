# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import inspect
import math
import os
import socket
import subprocess
import sys
import time
import typing

from . import capabilities, encoding


# Sometimes it's really hard to get Python extensions to compile,
# so fall back to a pure Python implementation.
try:
    from . import bser

    # Demandimport causes modules to be loaded lazily. Force the load now
    # so that we can fall back on pybser if bser doesn't exist
    # pyre-ignore
    bser.pdu_info
except ImportError:
    from . import pybser as bser


bser: typing.Any


if os.name == "nt":
    import ctypes
    from ctypes import wintypes

    from .windows import (
        CancelIoEx,
        CloseHandle,
        CreateEvent,
        CreateFile,
        ERROR_IO_PENDING,
        FILE_FLAG_OVERLAPPED,
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        FORMAT_MESSAGE_FROM_SYSTEM,
        FORMAT_MESSAGE_IGNORE_INSERTS,
        FormatMessage,
        GENERIC_READ,
        GENERIC_WRITE,
        GetLastError,
        GetOverlappedResult,
        GetOverlappedResultEx,
        INVALID_HANDLE_VALUE,
        LocalFree,
        OPEN_EXISTING,
        OVERLAPPED,
        ReadFile,
        SetLastError,
        WAIT_FAILED,
        WAIT_IO_COMPLETION,
        WAIT_OBJECT_0,
        WAIT_TIMEOUT,
        WaitForSingleObjectEx,
        WindowsSocketException,
        WindowsSocketHandle,
        WriteFile,
    )

# 2 bytes marker, 1 byte int size, 8 bytes int64 value
sniff_len = 13

# This is a helper for debugging the client.
_debugging = False
if _debugging:

    def log(fmt, *args):
        print(
            "[%s] %s"
            % (time.strftime("%a, %d %b %Y %H:%M:%S", time.gmtime()), fmt % args[:]),
            file=sys.stderr,
        )

else:

    def log(fmt, *args):
        pass


def _win32_strerror(err):
    """expand a win32 error code into a human readable message"""

    # FormatMessage will allocate memory and assign it here
    buf = ctypes.c_char_p()
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        None,
        err,
        0,
        buf,
        0,
        None,
    )
    try:
        return buf.value
    finally:
        LocalFree(buf)


class WatchmanError(Exception):
    def __init__(self, msg=None, cmd=None):
        self.msg = msg
        self.cmd = cmd

    def setCommand(self, cmd):
        self.cmd = cmd

    def __str__(self):
        if self.cmd:
            return "%s, while executing %s" % (self.msg, self.cmd)
        return self.msg


class BSERv1Unsupported(WatchmanError):
    pass


class UseAfterFork(WatchmanError):
    pass


class WatchmanEnvironmentError(WatchmanError):
    def __init__(self, msg, errno, errmsg, cmd=None):
        super(WatchmanEnvironmentError, self).__init__(
            "{0}: errno={1} errmsg={2}".format(msg, errno, errmsg), cmd
        )


class SocketConnectError(WatchmanError):
    def __init__(self, sockpath, exc):
        super(SocketConnectError, self).__init__(
            "unable to connect to %s: %s" % (sockpath, exc)
        )
        self.sockpath = sockpath
        self.exc = exc


class SocketTimeout(WatchmanError):
    """A specialized exception raised for socket timeouts during communication to/from watchman.
    This makes it easier to implement non-blocking loops as callers can easily distinguish
    between a routine timeout and an actual error condition.

    Note that catching WatchmanError will also catch this as it is a super-class, so backwards
    compatibility in exception handling is preserved.
    """


class CommandError(WatchmanError):
    """error returned by watchman

    self.msg is the message returned by watchman.
    """

    def __init__(self, msg, cmd=None):
        super(CommandError, self).__init__("watchman command error: %s" % (msg,), cmd)


def is_named_pipe_path(path: str) -> bool:
    """Returns True if path is a watchman named pipe path"""
    return path.startswith("\\\\.\\pipe\\watchman")


class SockPath:
    """Describes how to connect to watchman"""

    unix_domain = None
    named_pipe = None
    tcp_address = None

    def __init__(
        self, unix_domain=None, named_pipe=None, sockpath=None, tcp_address=None
    ):
        if named_pipe is None and sockpath is not None and is_named_pipe_path(sockpath):
            named_pipe = sockpath

        if (
            unix_domain is None
            and sockpath is not None
            and not is_named_pipe_path(sockpath)
        ):
            unix_domain = sockpath

        self.unix_domain = unix_domain
        self.named_pipe = named_pipe
        self.tcp_address = tcp_address

    def legacy_sockpath(self):
        """Returns a sockpath suitable for passing to the watchman
        CLI --sockname parameter"""
        log("legacy_sockpath called: %r", self)
        if os.name == "nt":
            return self.named_pipe
        return self.unix_domain


class Transport:
    """communication transport to the watchman server"""

    buf = None

    def close(self):
        """tear it down"""
        raise NotImplementedError()

    def readBytes(self, size):
        """read size bytes"""
        raise NotImplementedError()

    def write(self, buf):
        """write some data"""
        raise NotImplementedError()

    def setTimeout(self, value):
        pass

    def readLine(self):
        """read a line
        Maintains its own buffer, callers of the transport should not mix
        calls to readBytes and readLine.
        """
        if self.buf is None:
            self.buf = []

        # Buffer may already have a line if we've received unilateral
        # response(s) from the server
        if len(self.buf) == 1 and b"\n" in self.buf[0]:
            (line, b) = self.buf[0].split(b"\n", 1)
            self.buf = [b]
            return line

        while True:
            b = self.readBytes(4096)
            if b"\n" in b:
                result = b"".join(self.buf)
                (line, b) = b.split(b"\n", 1)
                self.buf = [b]
                return result + line
            self.buf.append(b)


class Codec:
    """communication encoding for the watchman server"""

    transport = None

    def __init__(self, transport):
        self.transport = transport

    def receive(self):
        raise NotImplementedError()

    def send(self, *args):
        raise NotImplementedError()

    def setTimeout(self, value):
        self.transport.setTimeout(value)


class SocketTransport(Transport):
    """abstract socket transport"""

    sock = None
    timeout = None

    def __init__(self):
        pass

    def close(self):
        if self.sock:
            self.sock.close()
            self.sock = None

    def setTimeout(self, value):
        self.timeout = value
        self.sock.settimeout(self.timeout)

    def readBytes(self, size):
        try:
            buf = [self.sock.recv(size)]
            if not buf[0]:
                raise WatchmanError("empty watchman response")
            return buf[0]
        except socket.timeout:
            raise SocketTimeout("timed out waiting for response")

    def write(self, data):
        try:
            log("write %r", data)
            self.sock.sendall(data)
        except socket.timeout:
            raise SocketTimeout("timed out sending query command")


class UnixSocketTransport(SocketTransport):
    """local unix domain socket transport"""

    def __init__(self, sockpath, timeout):
        super(UnixSocketTransport, self).__init__()
        self.sockpath = sockpath
        self.timeout = timeout

        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        try:
            sock.settimeout(self.timeout)
            sock.connect(self.sockpath.unix_domain)
            self.sock = sock
        except socket.error as e:
            sock.close()
            raise SocketConnectError(self.sockpath.unix_domain, e)


class WindowsUnixSocketTransport(SocketTransport):
    """local unix domain socket transport on Windows"""

    sock = None
    timeout = None

    def __init__(self, sockpath, timeout):
        super(WindowsUnixSocketTransport, self).__init__()
        self.sockpath = sockpath
        self.timeout = timeout

        sock = None
        try:
            sock = WindowsSocketHandle()
            sock.settimeout(self.timeout)
            sock.connect(self.sockpath.unix_domain)
            self.sock = sock
        except WindowsSocketException as e:
            if sock is not None:
                sock.close()
            raise SocketConnectError(self.sockpath.unix_domain, e)


class TcpSocketTransport(SocketTransport):
    """TCP socket transport"""

    def __init__(self, sockpath, timeout):
        super(TcpSocketTransport, self).__init__()
        self.sockpath = sockpath
        self.timeout = timeout

        try:
            # Note that address resolution does not respect 'timeout'
            # which applies only to socket traffic.
            results = socket.getaddrinfo(
                self.sockpath.tcp_address[0],
                self.sockpath.tcp_address[1],
                0,
                socket.IPPROTO_TCP,
            )
            (family, type, proto, canonname, sockaddr) = results[0]
        except Exception as e:
            raise SocketConnectError(
                "Error resolving address: %r" % (self.sockpath.tcp_address,), e
            )

        sock = socket.socket(family, socket.SOCK_STREAM)
        try:
            sock.settimeout(self.timeout)
            sock.connect(sockaddr)
            self.sock = sock
        except socket.error as e:
            sock.close()
            raise SocketConnectError(str(self.sockpath.tcp_address), e)


def _get_overlapped_result_ex_impl(pipe, olap, nbytes, millis, alertable):
    """Windows 7 and earlier does not support GetOverlappedResultEx. The
    alternative is to use GetOverlappedResult and wait for read or write
    operation to complete. This is done be using CreateEvent and
    WaitForSingleObjectEx. CreateEvent, WaitForSingleObjectEx
    and GetOverlappedResult are all part of Windows API since WindowsXP.
    This is the exact same implementation that can be found in the watchman
    source code (see get_overlapped_result_ex_impl in stream_win.c). This
    way, maintenance should be simplified.
    """
    log("Preparing to wait for maximum %dms", millis)
    if millis != 0:
        waitReturnCode = WaitForSingleObjectEx(olap.hEvent, millis, alertable)
        if waitReturnCode == WAIT_OBJECT_0:
            # Event is signaled, overlapped IO operation result should be available.
            pass
        elif waitReturnCode == WAIT_IO_COMPLETION:
            # WaitForSingleObjectEx returnes because the system added an I/O completion
            # routine or an asynchronous procedure call (APC) to the thread queue.
            SetLastError(WAIT_IO_COMPLETION)
            pass
        elif waitReturnCode == WAIT_TIMEOUT:
            # We reached the maximum allowed wait time, the IO operation failed
            # to complete in timely fashion.
            SetLastError(WAIT_TIMEOUT)
            return False
        elif waitReturnCode == WAIT_FAILED:
            # something went wrong calling WaitForSingleObjectEx
            err = GetLastError()
            log("WaitForSingleObjectEx failed: %s", _win32_strerror(err))
            return False
        else:
            # unexpected situation deserving investigation.
            err = GetLastError()
            log("Unexpected error: %s", _win32_strerror(err))
            return False

    return GetOverlappedResult(pipe, olap, nbytes, False)


class WindowsNamedPipeTransport(Transport):
    """connect to a named pipe"""

    def __init__(self, sockpath, timeout):
        self.sockpath = sockpath
        self.timeout = int(math.ceil(timeout * 1000))
        self._iobuf = None

        path = os.fsencode(self.sockpath.named_pipe)

        log("CreateFile %r", path)

        self.pipe = CreateFile(
            path,
            GENERIC_READ | GENERIC_WRITE,
            0,
            None,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            None,
        )

        err = GetLastError()
        if self.pipe == INVALID_HANDLE_VALUE or self.pipe == 0:
            self.pipe = None
            raise SocketConnectError(
                self.sockpath.named_pipe, self._make_win_err("", err)
            )

        # event for the overlapped I/O operations
        self._waitable = CreateEvent(None, True, False, None)
        err = GetLastError()
        if self._waitable is None:
            self._raise_win_err("CreateEvent failed", err)

        self._get_overlapped_result_ex = GetOverlappedResultEx
        if (
            os.getenv("WATCHMAN_WIN7_COMPAT") == "1"
            or self._get_overlapped_result_ex is None
        ):
            self._get_overlapped_result_ex = _get_overlapped_result_ex_impl

    def _raise_win_err(self, msg, err):
        raise self._make_win_err(msg, err)

    def _make_win_err(self, msg, err):
        return IOError("%s win32 error code: %d %s" % (msg, err, _win32_strerror(err)))

    def close(self):
        if self.pipe:
            log("Closing pipe")
            CloseHandle(self.pipe)
        self.pipe = None

        if self._waitable is not None:
            # We release the handle for the event
            CloseHandle(self._waitable)
        self._waitable = None

    def setTimeout(self, value):
        # convert to milliseconds
        self.timeout = int(value * 1000)

    def readBytes(self, size):
        """A read can block for an unbounded amount of time, even if the
        kernel reports that the pipe handle is signalled, so we need to
        always perform our reads asynchronously
        """

        # try to satisfy the read from any buffered data
        if self._iobuf:
            if size >= len(self._iobuf):
                res = self._iobuf
                self.buf = None
                return res
            res = self._iobuf[:size]
            self._iobuf = self._iobuf[size:]
            return res

        # We need to initiate a read
        buf = ctypes.create_string_buffer(size)
        olap = OVERLAPPED()
        olap.hEvent = self._waitable

        log("made read buff of size %d", size)

        # ReadFile docs warn against sending in the nread parameter for async
        # operations, so we always collect it via GetOverlappedResultEx
        immediate = ReadFile(self.pipe, buf, size, None, olap)

        if not immediate:
            err = GetLastError()
            if err != ERROR_IO_PENDING:
                self._raise_win_err("failed to read %d bytes" % size, err)

        nread = wintypes.DWORD()
        if not self._get_overlapped_result_ex(
            self.pipe, olap, nread, 0 if immediate else self.timeout, True
        ):
            err = GetLastError()
            CancelIoEx(self.pipe, olap)

            if err == WAIT_TIMEOUT:
                log("GetOverlappedResultEx timedout")
                raise SocketTimeout(
                    "timed out after waiting %dms for read" % self.timeout
                )

            log("GetOverlappedResultEx reports error %d", err)
            self._raise_win_err("error while waiting for read", err)

        nread = nread.value
        if nread == 0:
            # Docs say that named pipes return 0 byte when the other end did
            # a zero byte write.  Since we don't ever do that, the only
            # other way this shows up is if the client has gotten in a weird
            # state, so let's bail out
            CancelIoEx(self.pipe, olap)
            raise IOError("Async read yielded 0 bytes; unpossible!")

        # Holds precisely the bytes that we read from the prior request
        buf = buf[:nread]

        returned_size = min(nread, size)
        if returned_size == nread:
            return buf

        # keep any left-overs around for a later read to consume
        self._iobuf = buf[returned_size:]
        return buf[:returned_size]

    def write(self, data):
        olap = OVERLAPPED()
        olap.hEvent = self._waitable

        immediate = WriteFile(self.pipe, ctypes.c_char_p(data), len(data), None, olap)

        if not immediate:
            err = GetLastError()
            if err != ERROR_IO_PENDING:
                self._raise_win_err(
                    "failed to write %d bytes to handle %r" % (len(data), self.pipe),
                    err,
                )

        # Obtain results, waiting if needed
        nwrote = wintypes.DWORD()
        if self._get_overlapped_result_ex(
            self.pipe, olap, nwrote, 0 if immediate else self.timeout, True
        ):
            log("made write of %d bytes", nwrote.value)
            return nwrote.value

        err = GetLastError()

        # It's potentially unsafe to allow the write to continue after
        # we unwind, so let's make a best effort to avoid that happening
        CancelIoEx(self.pipe, olap)

        if err == WAIT_TIMEOUT:
            raise SocketTimeout("timed out after waiting %dms for write" % self.timeout)
        self._raise_win_err(
            "error while waiting for write of %d bytes" % len(data), err
        )


def _default_binpath(binpath=None) -> str:
    if binpath:
        return binpath
    # The test harness sets WATCHMAN_BINARY to the binary under test,
    # so we use that by default, otherwise, allow resolving watchman
    # from the users PATH.
    return os.environ.get("WATCHMAN_BINARY", "watchman")


class CLIProcessTransport(Transport):
    """open a pipe to the cli to talk to the service
    This intended to be used only in the test harness!

    The CLI is an oddball because we only support JSON input
    and cannot send multiple commands through the same instance,
    so we spawn a new process for each command.

    We disable server spawning for this implementation, again, because
    it is intended to be used only in our test harness.  You really
    should not need to use the CLI transport for anything real.

    While the CLI can output in BSER, our Transport interface doesn't
    support telling this instance that it should do so.  That effectively
    limits this implementation to JSON input and output only at this time.

    It is the responsibility of the caller to set the send and
    receive codecs appropriately.
    """

    proc = None
    closed = True

    def __init__(self, sockpath, timeout, binpath=None):
        self.sockpath = sockpath
        self.timeout = timeout
        self.binpath = _default_binpath(binpath)

    def close(self):
        if self.proc:
            if self.proc.pid is not None:
                self.proc.kill()
            self.proc.stdin.close()
            self.proc.stdout.close()
            self.proc.wait()
            self.proc = None

    def _connect(self):
        if self.proc:
            return self.proc
        args = [
            self.binpath,
            "--unix-listener-path={0}".format(self.sockpath.unix_domain),
            "--named-pipe-path={0}".format(self.sockpath.named_pipe),
            "--logfile=/BOGUS",
            "--statefile=/BOGUS",
            "--no-spawn",
            "--no-local",
            "--no-pretty",
            "-j",
        ]
        log("starting with %r", args)
        self.proc = subprocess.Popen(
            args, stdin=subprocess.PIPE, stdout=subprocess.PIPE
        )
        return self.proc

    def readBytes(self, size):
        self._connect()
        res = self.proc.stdout.read(size)
        log("CLI read %r", repr(res))
        if not res:
            raise WatchmanError("EOF on CLI process transport")
        return res

    def write(self, data):
        if self.closed:
            self.close()
            self.closed = False
        self._connect()
        log("CLI write %r", data)
        res = self.proc.stdin.write(data)
        self.proc.stdin.close()
        self.closed = True
        return res


class BserCodec(Codec):
    """use the BSER encoding.  This is the default, preferred codec"""

    def __init__(self, transport, value_encoding, value_errors):
        super(BserCodec, self).__init__(transport)
        self._value_encoding = value_encoding
        self._value_errors = value_errors

    def _loads(self, response):
        return bser.loads(
            response,
            value_encoding=self._value_encoding,
            value_errors=self._value_errors,
        )

    def receive(self):
        buf = [self.transport.readBytes(sniff_len)]
        if not buf[0]:
            raise WatchmanError("empty watchman response")

        _1, _2, elen = bser.pdu_info(buf[0])

        rlen = len(buf[0])
        while elen > rlen:
            buf.append(self.transport.readBytes(elen - rlen))
            rlen += len(buf[-1])

        response = b"".join(buf)
        try:
            res = self._loads(response)
            return res
        except ValueError as e:
            raise WatchmanError("watchman response decode error: %s" % e)

    def send(self, *args):
        cmd = bser.dumps(*args)  # Defaults to BSER v1
        self.transport.write(cmd)


class ImmutableBserCodec(BserCodec):
    """use the BSER encoding, decoding values using the newer
    immutable object support"""

    def _loads(self, response):
        return bser.loads(
            response,
            False,
            value_encoding=self._value_encoding,
            value_errors=self._value_errors,
        )


class Bser2WithFallbackCodec(BserCodec):
    """use BSER v2 encoding"""

    def __init__(self, transport, value_encoding, value_errors):
        super(Bser2WithFallbackCodec, self).__init__(
            transport, value_encoding, value_errors
        )
        bserv2_key = "required"

        self.send(["version", {bserv2_key: ["bser-v2"]}])

        capabilities = self.receive()

        if "error" in capabilities:
            raise BSERv1Unsupported(
                "The watchman server version does not support Python 3. Please "
                "upgrade your watchman server."
            )

        if capabilities["capabilities"]["bser-v2"]:
            self.bser_version = 2
            self.bser_capabilities = 0
        else:
            self.bser_version = 1
            self.bser_capabilities = 0

    def receive(self):
        buf = [self.transport.readBytes(sniff_len)]
        if not buf[0]:
            raise WatchmanError("empty watchman response")

        recv_bser_version, recv_bser_capabilities, elen = bser.pdu_info(buf[0])

        if hasattr(self, "bser_version"):
            # Readjust BSER version and capabilities if necessary
            self.bser_version = max(self.bser_version, recv_bser_version)
            self.capabilities = self.bser_capabilities & recv_bser_capabilities

        rlen = len(buf[0])
        while elen > rlen:
            buf.append(self.transport.readBytes(elen - rlen))
            rlen += len(buf[-1])

        response = b"".join(buf)
        try:
            res = self._loads(response)
            return res
        except ValueError as e:
            raise WatchmanError("watchman response decode error: %s" % e)

    def send(self, *args):
        if hasattr(self, "bser_version"):
            cmd = bser.dumps(
                *args, version=self.bser_version, capabilities=self.bser_capabilities
            )
        else:
            cmd = bser.dumps(*args)
        self.transport.write(cmd)


class ImmutableBser2Codec(Bser2WithFallbackCodec, ImmutableBserCodec):
    """use the BSER encoding, decoding values using the newer
    immutable object support"""

    pass


class JsonCodec(Codec):
    """Use json codec.  This is here primarily for testing purposes"""

    json = None

    def __init__(self, transport):
        super(JsonCodec, self).__init__(transport)
        # optional dep on json, only if JsonCodec is used
        import json

        self.json = json

    def receive(self):
        line = self.transport.readLine()
        try:
            # In Python 3, json.loads is a transformation from Unicode string to
            # objects possibly containing Unicode strings. We typically expect
            # the JSON blob to be ASCII-only with non-ASCII characters escaped,
            # but it's possible we might get non-ASCII bytes that are valid
            # UTF-8.
            line = line.decode("utf-8")
            return self.json.loads(line)
        except Exception as e:
            print(e, line)
            raise

    def send(self, *args):
        cmd = self.json.dumps(*args)
        # In Python 3, json.dumps is a transformation from objects possibly
        # containing Unicode strings to Unicode string. Even with (the default)
        # ensure_ascii=True, dumps returns a Unicode string.
        cmd = cmd.encode("ascii")
        self.transport.write(cmd + b"\n")


class client:
    """Handles the communication with the watchman service"""

    sockpath = None
    transport = None
    sendCodec = None
    recvCodec = None
    sendConn = None
    recvConn = None
    subs = {}  # Keyed by subscription name
    sub_by_root = {}  # Keyed by root, then by subscription name
    logs = []  # When log level is raised
    unilateral = ["log", "subscription"]
    tport = None
    useImmutableBser = None
    pid = None

    def __init__(
        self,
        sockpath=None,
        tcpAddress=None,
        timeout=1.0,
        transport=None,
        sendEncoding=None,
        recvEncoding=None,
        useImmutableBser=False,
        # use False for these two because None has a special
        # meaning
        valueEncoding=False,
        valueErrors=False,
        binpath=None,
    ):
        if sockpath is not None and not isinstance(sockpath, SockPath):
            sockpath = SockPath(sockpath=sockpath, tcp_address=tcpAddress)
        self.sockpath = sockpath
        self.timeout = timeout
        self.useImmutableBser = useImmutableBser
        self.binpath = _default_binpath(binpath)

        if inspect.isclass(transport) and issubclass(transport, Transport):
            self.transport = transport
        else:
            log(
                "figure out transport. param=%r, env=%r",
                transport,
                os.getenv("WATCHMAN_TRANSPORT"),
            )
            transport = transport or os.getenv("WATCHMAN_TRANSPORT") or "local"
            if self.transport == "tcp" and tcpAddress is None:
                raise WatchmanError(
                    "Constructor requires argument tcpAddress when 'tcp' "
                    "transport protocol is used"
                )
            if (transport == "namedpipe") or (transport == "local" and os.name == "nt"):
                self.transport = WindowsNamedPipeTransport
            elif transport == "unix" and os.name == "nt":
                self.transport = WindowsUnixSocketTransport
            elif transport == "local" or transport == "unix":
                self.transport = UnixSocketTransport
            elif transport == "cli":
                self.transport = CLIProcessTransport
                if sendEncoding is None:
                    sendEncoding = "json"
                if recvEncoding is None:
                    recvEncoding = sendEncoding
            elif transport == "tcp":
                self.transport = TcpSocketTransport
            else:
                raise WatchmanError("invalid transport %s" % transport)

        sendEncoding = str(sendEncoding or os.getenv("WATCHMAN_ENCODING") or "bser")
        recvEncoding = str(recvEncoding or os.getenv("WATCHMAN_ENCODING") or "bser")

        self.recvCodec = self._parseEncoding(recvEncoding)
        self.sendCodec = self._parseEncoding(sendEncoding)

        # We want to act like the native OS methods as much as possible. This
        # means returning bytestrings on Python 2 by default and Unicode
        # strings on Python 3. However we take an optional argument that lets
        # users override this.
        if valueEncoding is False:
            self.valueEncoding = encoding.get_local_encoding()
            self.valueErrors = encoding.default_local_errors
        else:
            self.valueEncoding = valueEncoding
            if valueErrors is False:
                self.valueErrors = encoding.default_local_errors
            else:
                self.valueErrors = valueErrors

    def _makeBSERCodec(self, codec):
        def make_codec(transport):
            return codec(transport, self.valueEncoding, self.valueErrors)

        return make_codec

    def _parseEncoding(self, enc):
        if enc == "bser":
            if self.useImmutableBser:
                return self._makeBSERCodec(ImmutableBser2Codec)
            return self._makeBSERCodec(Bser2WithFallbackCodec)
        elif enc == "bser-v1":
            raise BSERv1Unsupported(
                "Python 3 does not support the BSER v1 encoding: specify "
                '"bser" or omit the sendEncoding and recvEncoding '
                "arguments"
            )
        elif enc == "json":
            return JsonCodec
        else:
            raise WatchmanError("invalid encoding %s" % enc)

    def _hasprop(self, result, name):
        if self.useImmutableBser:
            return hasattr(result, name)
        return name in result

    def _resolvesockname(self):
        # if invoked via a trigger, watchman will set this env var; we
        # should use it unless explicitly set otherwise
        path = os.getenv("WATCHMAN_SOCK")
        if path:
            return SockPath(sockpath=path)

        cmd = [self.binpath, "--output-encoding=bser", "get-sockname"]
        try:
            args = dict(stdout=subprocess.PIPE, stderr=subprocess.PIPE)  # noqa: C408

            if os.name == "nt":
                # if invoked via an application with graphical user interface,
                # this call will cause a brief command window pop-up.
                # Using the flag STARTF_USESHOWWINDOW to avoid this behavior.
                startupinfo = subprocess.STARTUPINFO()
                startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
                args["startupinfo"] = startupinfo

            p = subprocess.Popen(cmd, **args)

        except OSError as e:
            raise WatchmanError('"watchman" executable not in PATH (%s)', e)

        stdout, stderr = p.communicate()
        exitcode = p.poll()

        if exitcode:
            raise WatchmanError("watchman exited with code %d" % exitcode)

        result = bser.loads(stdout)
        if "error" in result:
            raise WatchmanError("get-sockname error: %s" % result["error"])

        def get_path_result(name):
            value = result.get(name, None)
            if value is None:
                return None
            return value.decode(sys.getfilesystemencoding(), errors="surrogateescape")

        # sockname is always present
        sockpath = get_path_result("sockname")
        assert sockpath is not None

        return SockPath(
            # unix_domain and named_pipe are reported by newer versions
            # of the server and may not be present
            unix_domain=get_path_result("unix_domain"),
            named_pipe=get_path_result("named_pipe"),
            # sockname is always present
            sockpath=sockpath,
        )

    def _connect(self):
        """establish transport connection"""

        if self.recvConn:
            if self.pid != os.getpid():
                raise UseAfterFork(
                    "do not re-use a connection after fork; open a new client instead"
                )
            return

        if self.sockpath is None:
            self.sockpath = self._resolvesockname()

        kwargs = {}
        if self.transport == CLIProcessTransport:
            kwargs["binpath"] = self.binpath

        self.tport = self.transport(self.sockpath, self.timeout, **kwargs)
        self.sendConn = self.sendCodec(self.tport)
        self.recvConn = self.recvCodec(self.tport)
        self.pid = os.getpid()

    def __del__(self):
        self.close()

    def __enter__(self):
        self._connect()
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.close()

    def close(self):
        if self.tport:
            self.tport.close()
            self.tport = None
            self.recvConn = None
            self.sendConn = None

    def receive(self):
        """receive the next PDU from the watchman service

        If the client has activated subscriptions or logs then
        this PDU may be a unilateral PDU sent by the service to
        inform the client of a log event or subscription change.

        It may also simply be the response portion of a request
        initiated by query.

        There are clients in production that subscribe and call
        this in a loop to retrieve all subscription responses,
        so care should be taken when making changes here.
        """

        self._connect()
        result = self.recvConn.receive()
        if self._hasprop(result, "error"):
            raise CommandError(result["error"])

        if self._hasprop(result, "log"):
            self.logs.append(result["log"])

        if self._hasprop(result, "subscription"):
            sub = result["subscription"]
            if not (sub in self.subs):
                self.subs[sub] = []
            self.subs[sub].append(result)

            # also accumulate in {root,sub} keyed store
            root = os.path.normpath(os.path.normcase(result["root"]))
            if not root in self.sub_by_root:
                self.sub_by_root[root] = {}
            if not sub in self.sub_by_root[root]:
                self.sub_by_root[root][sub] = []
            self.sub_by_root[root][sub].append(result)

        return result

    def isUnilateralResponse(self, res):
        if "unilateral" in res and res["unilateral"]:
            return True
        # Fall back to checking for known unilateral responses
        for k in self.unilateral:
            if k in res:
                return True
        return False

    def getLog(self, remove=True):
        """Retrieve buffered log data

        If remove is true the data will be removed from the buffer.
        Otherwise it will be left in the buffer
        """
        res = self.logs
        if remove:
            self.logs = []
        return res

    def getSubscription(self, name, remove=True, root=None):
        """Retrieve the data associated with a named subscription

        If remove is True (the default), the subscription data is removed
        from the buffer.  Otherwise the data is returned but left in
        the buffer.

        Returns None if there is no data associated with `name`

        If root is not None, then only return the subscription
        data that matches both root and name.  When used in this way,
        remove processing impacts both the unscoped and scoped stores
        for the subscription data.
        """
        if root is not None:
            root = os.path.normpath(os.path.normcase(root))
            if root not in self.sub_by_root:
                return None
            if name not in self.sub_by_root[root]:
                return None
            sub = self.sub_by_root[root][name]
            if remove:
                del self.sub_by_root[root][name]
                # don't let this grow unbounded
                if name in self.subs:
                    del self.subs[name]
            return sub

        if name not in self.subs:
            return None
        sub = self.subs[name]
        if remove:
            del self.subs[name]
        return sub

    def query(self, *args):
        """Send a query to the watchman service and return the response

        This call will block until the response is returned.
        If any unilateral responses are sent by the service in between
        the request-response they will be buffered up in the client object
        and NOT returned via this method.
        """

        log("calling client.query")
        self._connect()
        try:
            self.sendConn.send(args)

            res = self.receive()
            while self.isUnilateralResponse(res):
                res = self.receive()

            return res
        except EnvironmentError as ee:
            # When we can depend on Python 3, we can use PEP 3134
            # exception chaining here.
            raise WatchmanEnvironmentError(
                "I/O error communicating with watchman daemon",
                ee.errno,
                ee.strerror,
                args,
            )
        except WatchmanError as ex:
            ex.setCommand(args)
            raise

    def capabilityCheck(self, optional=None, required=None):
        """Perform a server capability check"""
        opts = {"optional": optional or [], "required": required or []}
        res = self.query("version", opts)

        if not self._hasprop(res, "capabilities"):
            # Server doesn't support capabilities, so we need to
            # synthesize the results based on the version
            capabilities.synthesize(res, opts)
            if "error" in res:
                raise CommandError(res["error"])

        return res

    def listCapabilities(self):
        return self.query("list-capabilities", {})["capabilities"]

    def setTimeout(self, value):
        self.recvConn.setTimeout(value)
        self.sendConn.setTimeout(value)
