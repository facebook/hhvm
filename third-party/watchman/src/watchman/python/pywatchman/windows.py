# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import ctypes
import ctypes.wintypes
import os
import socket


# pyre-ignore
wintypes = ctypes.wintypes
GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000
FILE_FLAG_OVERLAPPED = 0x40000000
OPEN_EXISTING = 3
INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value
FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000
FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100
FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200
WAIT_FAILED = 0xFFFFFFFF
WAIT_TIMEOUT = 0x00000102
WAIT_OBJECT_0 = 0x00000000
WAIT_IO_COMPLETION = 0x000000C0
INFINITE = 0xFFFFFFFF
SO_SNDTIMEO = 0x1005
SO_RCVTIMEO = 0x1006
SOL_SOCKET = 0xFFFF
WSAETIMEDOUT = 10060

# Overlapped I/O operation is in progress. (997)
ERROR_IO_PENDING = 0x000003E5

# The pointer size follows the architecture
# We use WPARAM since this type is already conditionally defined
ULONG_PTR = ctypes.wintypes.WPARAM


class OVERLAPPED(ctypes.Structure):
    _fields_ = [
        ("Internal", ULONG_PTR),
        ("InternalHigh", ULONG_PTR),
        ("Offset", wintypes.DWORD),
        ("OffsetHigh", wintypes.DWORD),
        ("hEvent", wintypes.HANDLE),
    ]

    def __init__(self):
        self.Internal = 0
        self.InternalHigh = 0
        self.Offset = 0
        self.OffsetHigh = 0
        self.hEvent = 0


LPDWORD = ctypes.POINTER(wintypes.DWORD)

# pyre-ignore
windll = ctypes.windll

CreateFile = windll.kernel32.CreateFileA
CreateFile.argtypes = [
    wintypes.LPSTR,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.LPVOID,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.HANDLE,
]
CreateFile.restype = wintypes.HANDLE

CloseHandle = windll.kernel32.CloseHandle
CloseHandle.argtypes = [wintypes.HANDLE]
CloseHandle.restype = wintypes.BOOL

ReadFile = windll.kernel32.ReadFile
ReadFile.argtypes = [
    wintypes.HANDLE,
    wintypes.LPVOID,
    wintypes.DWORD,
    LPDWORD,
    ctypes.POINTER(OVERLAPPED),
]
ReadFile.restype = wintypes.BOOL

WriteFile = windll.kernel32.WriteFile
WriteFile.argtypes = [
    wintypes.HANDLE,
    wintypes.LPVOID,
    wintypes.DWORD,
    LPDWORD,
    ctypes.POINTER(OVERLAPPED),
]
WriteFile.restype = wintypes.BOOL

GetLastError = windll.kernel32.GetLastError
GetLastError.argtypes = []
GetLastError.restype = wintypes.DWORD

SetLastError = windll.kernel32.SetLastError
SetLastError.argtypes = [wintypes.DWORD]
SetLastError.restype = None

FormatMessage = windll.kernel32.FormatMessageA
FormatMessage.argtypes = [
    wintypes.DWORD,
    wintypes.LPVOID,
    wintypes.DWORD,
    wintypes.DWORD,
    ctypes.POINTER(wintypes.LPSTR),
    wintypes.DWORD,
    wintypes.LPVOID,
]
FormatMessage.restype = wintypes.DWORD

LocalFree = windll.kernel32.LocalFree

GetOverlappedResult = windll.kernel32.GetOverlappedResult
GetOverlappedResult.argtypes = [
    wintypes.HANDLE,
    ctypes.POINTER(OVERLAPPED),
    LPDWORD,
    wintypes.BOOL,
]
GetOverlappedResult.restype = wintypes.BOOL

GetOverlappedResultEx = getattr(windll.kernel32, "GetOverlappedResultEx", None)
if GetOverlappedResultEx is not None:
    GetOverlappedResultEx.argtypes = [
        wintypes.HANDLE,
        ctypes.POINTER(OVERLAPPED),
        LPDWORD,
        wintypes.DWORD,
        wintypes.BOOL,
    ]
    GetOverlappedResultEx.restype = wintypes.BOOL

WaitForSingleObjectEx = windll.kernel32.WaitForSingleObjectEx
WaitForSingleObjectEx.argtypes = [wintypes.HANDLE, wintypes.DWORD, wintypes.BOOL]
WaitForSingleObjectEx.restype = wintypes.DWORD

CreateEvent = windll.kernel32.CreateEventA
CreateEvent.argtypes = [LPDWORD, wintypes.BOOL, wintypes.BOOL, wintypes.LPSTR]
CreateEvent.restype = wintypes.HANDLE

# Windows Vista is the minimum supported client for CancelIoEx.
CancelIoEx = windll.kernel32.CancelIoEx
CancelIoEx.argtypes = [wintypes.HANDLE, ctypes.POINTER(OVERLAPPED)]
CancelIoEx.restype = wintypes.BOOL

WinSocket = windll.ws2_32.socket
WinSocket.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
WinSocket.restype = ctypes.wintypes.HANDLE

WinConnect = windll.ws2_32.connect
WinConnect.argtypes = [ctypes.wintypes.HANDLE, ctypes.c_void_p, ctypes.c_int]
WinConnect.restype = ctypes.c_int

WinSend = windll.ws2_32.send
WinSend.argtypes = [ctypes.wintypes.HANDLE, ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
WinSend.restype = ctypes.c_int

WinRecv = windll.ws2_32.recv
WinRecv.argtypes = [ctypes.wintypes.HANDLE, ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
WinRecv.restype = ctypes.c_int

closesocket = windll.ws2_32.closesocket
closesocket.argtypes = [ctypes.wintypes.HANDLE]
closesocket.restype = ctypes.c_int

WinSetIntSockOpt = windll.ws2_32.setsockopt
WinSetIntSockOpt.argtypes = [
    ctypes.wintypes.HANDLE,
    ctypes.c_int,
    ctypes.c_int,
    wintypes.LPDWORD,
    ctypes.c_int,
]
WinSetIntSockOpt.restype = ctypes.c_int

WSAGetLastError = windll.ws2_32.WSAGetLastError
WSAGetLastError.argtypes = []

WSADESCRIPTION_LEN = 256 + 1
WSASYS_STATUS_LEN = 128 + 1


class WSAData64(ctypes.Structure):
    _fields_ = [
        ("wVersion", ctypes.c_ushort),
        ("wHighVersion", ctypes.c_ushort),
        ("iMaxSockets", ctypes.c_ushort),
        ("iMaxUdpDg", ctypes.c_ushort),
        ("lpVendorInfo", ctypes.c_char_p),
        ("szDescription", ctypes.c_ushort * WSADESCRIPTION_LEN),
        ("szSystemStatus", ctypes.c_ushort * WSASYS_STATUS_LEN),
    ]


WSAStartup = windll.ws2_32.WSAStartup
WSAStartup.argtypes = [ctypes.wintypes.WORD, ctypes.POINTER(WSAData64)]
WSAStartup.restype = ctypes.c_int


class SOCKADDR_UN(ctypes.Structure):
    _fields_ = [("sun_family", ctypes.c_ushort), ("sun_path", ctypes.c_char * 108)]


class WindowsSocketException(Exception):
    def __init__(self, code: int) -> None:
        super(WindowsSocketException, self).__init__(
            "Windows Socket Error: {}".format(code)
        )


class WindowsSocketHandle:
    AF_UNIX = 1
    SOCK_STREAM = 1

    fd: int = -1
    address: str = ""

    @staticmethod
    def _checkReturnCode(retcode):
        if retcode == -1:
            errcode = WSAGetLastError()
            if errcode == WSAETIMEDOUT:
                raise socket.timeout()
            raise WindowsSocketException(errcode)

    def __init__(self):
        wsa_data = WSAData64()
        # ctypes.c_ushort(514) = MAKE_WORD(2,2) which is for the winsock
        # library version 2.2
        errcode = WSAStartup(ctypes.c_ushort(514), ctypes.pointer(wsa_data))
        if errcode != 0:
            raise WindowsSocketException(errcode)

        fd = WinSocket(self.AF_UNIX, self.SOCK_STREAM, 0)
        self._checkReturnCode(fd)
        self.fd = fd

    def fileno(self) -> int:
        return self.fd

    def settimeout(self, timeout: int) -> None:
        timeout = wintypes.DWORD(0 if timeout is None else int(timeout * 1000))
        retcode = WinSetIntSockOpt(
            self.fd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            ctypes.byref(timeout),
            ctypes.sizeof(timeout),
        )
        self._checkReturnCode(retcode)
        retcode = WinSetIntSockOpt(
            self.fd,
            SOL_SOCKET,
            SO_SNDTIMEO,
            ctypes.byref(timeout),
            ctypes.sizeof(timeout),
        )
        self._checkReturnCode(retcode)
        return None

    def connect(self, address: str) -> None:
        # pyre-ignore
        address = os.fsencode(os.path.normpath(address))
        addr = SOCKADDR_UN(sun_family=self.AF_UNIX, sun_path=address)
        self._checkReturnCode(
            WinConnect(self.fd, ctypes.pointer(addr), ctypes.sizeof(addr))
        )
        self.address = address

    def send(self, buff: bytes) -> int:
        retcode = WinSend(self.fd, buff, len(buff), 0)
        self._checkReturnCode(retcode)
        return retcode

    def sendall(self, buff: bytes) -> None:
        while len(buff) > 0:
            x = self.send(buff)
            if x > 0:
                buff = buff[x:]
            else:
                break
        return None

    def recv(self, size: int) -> bytes:
        buff = ctypes.create_string_buffer(size)
        retsize = WinRecv(self.fd, buff, size, 0)
        self._checkReturnCode(retsize)
        return buff.raw[0:retsize]

    def getpeername(self) -> str:
        return self.address

    def getsockname(self) -> str:
        return self.address

    def close(self) -> int:
        return closesocket(self.fd)
