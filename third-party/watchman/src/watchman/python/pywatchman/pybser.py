# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import binascii
import collections.abc as collections_abc
import ctypes
import struct
import sys


BSER_ARRAY = b"\x00"
BSER_OBJECT = b"\x01"
BSER_BYTESTRING = b"\x02"
BSER_INT8 = b"\x03"
BSER_INT16 = b"\x04"
BSER_INT32 = b"\x05"
BSER_INT64 = b"\x06"
BSER_REAL = b"\x07"
BSER_TRUE = b"\x08"
BSER_FALSE = b"\x09"
BSER_NULL = b"\x0a"
BSER_TEMPLATE = b"\x0b"
BSER_SKIP = b"\x0c"
BSER_UTF8STRING = b"\x0d"

STRING_TYPES = (str, bytes)
unicode = str


def tobytes(i):
    return str(i).encode("ascii")


long = int

# Leave room for the serialization header, which includes
# our overall length.  To make things simpler, we'll use an
# int32 for the header
EMPTY_HEADER = b"\x00\x01\x05\x00\x00\x00\x00"
EMPTY_HEADER_V2 = b"\x00\x02\x00\x00\x00\x00\x05\x00\x00\x00\x00"


def _int_size(x) -> int:
    """Return the smallest size int that can store the value"""
    if -0x80 <= x <= 0x7F:
        return 1
    elif -0x8000 <= x <= 0x7FFF:
        return 2
    elif -0x80000000 <= x <= 0x7FFFFFFF:
        return 4
    elif long(-0x8000000000000000) <= x <= long(0x7FFFFFFFFFFFFFFF):
        return 8
    else:
        raise RuntimeError("Cannot represent value: " + str(x))


def _buf_pos(buf, pos) -> bytes:
    ret = buf[pos]
    # Normalize the return type to bytes
    if not isinstance(ret, bytes):
        ret = bytes((ret,))
    return ret


class _bser_buffer(object):
    def __init__(self, version):
        self.bser_version = version
        self.buf = ctypes.create_string_buffer(8192)
        if self.bser_version == 1:
            struct.pack_into(
                tobytes(len(EMPTY_HEADER)) + b"s", self.buf, 0, EMPTY_HEADER
            )
            self.wpos = len(EMPTY_HEADER)
        else:
            assert self.bser_version == 2
            struct.pack_into(
                tobytes(len(EMPTY_HEADER_V2)) + b"s", self.buf, 0, EMPTY_HEADER_V2
            )
            self.wpos = len(EMPTY_HEADER_V2)

    def ensure_size(self, size):
        buf = self.buf
        old_size = ctypes.sizeof(buf)
        new_size = old_size
        while new_size - self.wpos < size:
            new_size *= 2
        if old_size != new_size:
            ctypes.resize(buf, new_size)

    def append_long(self, val):
        size = _int_size(val)
        to_write = size + 1
        self.ensure_size(to_write)
        if size == 1:
            struct.pack_into(b"=cb", self.buf, self.wpos, BSER_INT8, val)
        elif size == 2:
            struct.pack_into(b"=ch", self.buf, self.wpos, BSER_INT16, val)
        elif size == 4:
            struct.pack_into(b"=ci", self.buf, self.wpos, BSER_INT32, val)
        elif size == 8:
            struct.pack_into(b"=cq", self.buf, self.wpos, BSER_INT64, val)
        else:
            raise RuntimeError("Cannot represent this long value")
        self.wpos += to_write

    def append_string(self, s):
        if isinstance(s, unicode):
            s = s.encode("utf-8")
        s_len = len(s)
        size = _int_size(s_len)
        to_write = 2 + size + s_len
        self.ensure_size(to_write)
        if size == 1:
            struct.pack_into(
                b"=ccb" + tobytes(s_len) + b"s",
                self.buf,
                self.wpos,
                BSER_BYTESTRING,
                BSER_INT8,
                s_len,
                s,
            )
        elif size == 2:
            struct.pack_into(
                b"=cch" + tobytes(s_len) + b"s",
                self.buf,
                self.wpos,
                BSER_BYTESTRING,
                BSER_INT16,
                s_len,
                s,
            )
        elif size == 4:
            struct.pack_into(
                b"=cci" + tobytes(s_len) + b"s",
                self.buf,
                self.wpos,
                BSER_BYTESTRING,
                BSER_INT32,
                s_len,
                s,
            )
        elif size == 8:
            struct.pack_into(
                b"=ccq" + tobytes(s_len) + b"s",
                self.buf,
                self.wpos,
                BSER_BYTESTRING,
                BSER_INT64,
                s_len,
                s,
            )
        else:
            raise RuntimeError("Cannot represent this string value")
        self.wpos += to_write

    def append_recursive(self, val):
        if isinstance(val, bool):
            needed = 1
            self.ensure_size(needed)
            if val:
                to_encode = BSER_TRUE
            else:
                to_encode = BSER_FALSE
            struct.pack_into(b"=c", self.buf, self.wpos, to_encode)
            self.wpos += needed
        elif val is None:
            needed = 1
            self.ensure_size(needed)
            struct.pack_into(b"=c", self.buf, self.wpos, BSER_NULL)
            self.wpos += needed
        elif isinstance(val, (int, long)):
            self.append_long(val)
        elif isinstance(val, STRING_TYPES):
            self.append_string(val)
        elif isinstance(val, float):
            needed = 9
            self.ensure_size(needed)
            struct.pack_into(b"=cd", self.buf, self.wpos, BSER_REAL, val)
            self.wpos += needed
        elif isinstance(val, collections_abc.Mapping) and isinstance(
            val, collections_abc.Sized
        ):
            val_len = len(val)
            size = _int_size(val_len)
            needed = 2 + size
            self.ensure_size(needed)
            if size == 1:
                struct.pack_into(
                    b"=ccb", self.buf, self.wpos, BSER_OBJECT, BSER_INT8, val_len
                )
            elif size == 2:
                struct.pack_into(
                    b"=cch", self.buf, self.wpos, BSER_OBJECT, BSER_INT16, val_len
                )
            elif size == 4:
                struct.pack_into(
                    b"=cci", self.buf, self.wpos, BSER_OBJECT, BSER_INT32, val_len
                )
            elif size == 8:
                struct.pack_into(
                    b"=ccq", self.buf, self.wpos, BSER_OBJECT, BSER_INT64, val_len
                )
            else:
                raise RuntimeError("Cannot represent this mapping value")
            self.wpos += needed
            iteritems = val.items()
            for k, v in iteritems:
                self.append_string(k)
                self.append_recursive(v)
        elif isinstance(val, collections_abc.Iterable) and isinstance(
            val, collections_abc.Sized
        ):
            val_len = len(val)
            size = _int_size(val_len)
            needed = 2 + size
            self.ensure_size(needed)
            if size == 1:
                struct.pack_into(
                    b"=ccb", self.buf, self.wpos, BSER_ARRAY, BSER_INT8, val_len
                )
            elif size == 2:
                struct.pack_into(
                    b"=cch", self.buf, self.wpos, BSER_ARRAY, BSER_INT16, val_len
                )
            elif size == 4:
                struct.pack_into(
                    b"=cci", self.buf, self.wpos, BSER_ARRAY, BSER_INT32, val_len
                )
            elif size == 8:
                struct.pack_into(
                    b"=ccq", self.buf, self.wpos, BSER_ARRAY, BSER_INT64, val_len
                )
            else:
                raise RuntimeError("Cannot represent this sequence value")
            self.wpos += needed
            for v in val:
                self.append_recursive(v)
        else:
            raise RuntimeError("Cannot represent unknown value type")


def dumps(obj, version: int = 1, capabilities: int = 0):
    bser_buf = _bser_buffer(version=version)
    bser_buf.append_recursive(obj)
    # Now fill in the overall length
    if version == 1:
        obj_len = bser_buf.wpos - len(EMPTY_HEADER)
        try:
            struct.pack_into(b"=i", bser_buf.buf, 3, obj_len)
        except struct.error:
            # The C implementation treats overflow as MemoryError. Do the same here.
            raise MemoryError
    else:
        obj_len = bser_buf.wpos - len(EMPTY_HEADER_V2)
        struct.pack_into(b"=i", bser_buf.buf, 2, capabilities)
        struct.pack_into(b"=i", bser_buf.buf, 7, obj_len)
    return bser_buf.buf.raw[: bser_buf.wpos]


# This is a quack-alike with the bserObjectType in bser.c
# It provides by getattr accessors and getitem for both index
# and name.
class _BunserDict(object):
    __slots__ = ("_keys", "_values")

    def __init__(self, keys, values):
        self._keys = keys
        self._values = values

    def __getattr__(self, name):
        return self.__getitem__(name)

    def __getitem__(self, key):
        if isinstance(key, (int, long)):
            return self._values[key]
        elif key.startswith("st_"):
            # hack^Wfeature to allow mercurial to use "st_size" to
            # reference "size"
            key = key[3:]
        try:
            return self._values[self._keys.index(key)]
        except ValueError:
            raise KeyError("_BunserDict has no key %s" % key)

    def __len__(self):
        return len(self._keys)


class Bunser(object):
    def __init__(self, mutable=True, value_encoding=None, value_errors=None):
        self.mutable = mutable
        self.value_encoding = value_encoding

        if value_encoding is None:
            self.value_errors = None
        elif value_errors is None:
            self.value_errors = "strict"
        else:
            self.value_errors = value_errors

    @staticmethod
    def unser_int(buf, pos):
        try:
            int_type = _buf_pos(buf, pos)
        except IndexError:
            raise ValueError("Invalid bser int encoding, pos out of range")
        if int_type == BSER_INT8:
            needed = 2
            fmt = b"=b"
        elif int_type == BSER_INT16:
            needed = 3
            fmt = b"=h"
        elif int_type == BSER_INT32:
            needed = 5
            fmt = b"=i"
        elif int_type == BSER_INT64:
            needed = 9
            fmt = b"=q"
        else:
            raise ValueError(
                "Invalid bser int encoding 0x%s at position %s"
                % (binascii.hexlify(int_type).decode("ascii"), pos)
            )
        int_val = struct.unpack_from(fmt, buf, pos + 1)[0]
        return (int_val, pos + needed)

    def unser_utf8_string(self, buf, pos):
        str_len, pos = self.unser_int(buf, pos + 1)
        str_val = struct.unpack_from(tobytes(str_len) + b"s", buf, pos)[0]
        return (str_val.decode("utf-8"), pos + str_len)

    def unser_bytestring(self, buf, pos):
        str_len, pos = self.unser_int(buf, pos + 1)
        str_val = struct.unpack_from(tobytes(str_len) + b"s", buf, pos)[0]
        if self.value_encoding is not None:
            str_val = str_val.decode(self.value_encoding, self.value_errors)
            # str_len stays the same because that's the length in bytes
        return (str_val, pos + str_len)

    def unser_array(self, buf, pos):
        arr_len, pos = self.unser_int(buf, pos + 1)
        arr = []
        for _ in range(arr_len):
            arr_item, pos = self.loads_recursive(buf, pos)
            arr.append(arr_item)

        if not self.mutable:
            arr = tuple(arr)

        return arr, pos

    def unser_object(self, buf, pos):
        obj_len, pos = self.unser_int(buf, pos + 1)
        if self.mutable:
            obj = {}
        else:
            keys = []
            vals = []

        for _ in range(obj_len):
            key, pos = self.unser_utf8_string(buf, pos)
            val, pos = self.loads_recursive(buf, pos)
            if self.mutable:
                obj[key] = val
            else:
                keys.append(key)
                vals.append(val)

        if not self.mutable:
            obj = _BunserDict(keys, vals)

        return obj, pos

    def unser_template(self, buf, pos):
        val_type = _buf_pos(buf, pos + 1)
        if val_type != BSER_ARRAY:
            raise RuntimeError("Expect ARRAY to follow TEMPLATE")
        # force UTF-8 on keys
        keys_bunser = Bunser(mutable=self.mutable, value_encoding="utf-8")
        keys, pos = keys_bunser.unser_array(buf, pos + 1)
        nitems, pos = self.unser_int(buf, pos)
        arr = []
        for _ in range(nitems):
            if self.mutable:
                obj = {}
            else:
                vals = []

            for keyidx in range(len(keys)):
                if _buf_pos(buf, pos) == BSER_SKIP:
                    pos += 1
                    ele = None
                else:
                    ele, pos = self.loads_recursive(buf, pos)

                if self.mutable:
                    key = keys[keyidx]
                    obj[key] = ele
                else:
                    vals.append(ele)

            if not self.mutable:
                obj = _BunserDict(keys, vals)

            arr.append(obj)
        return arr, pos

    def loads_recursive(self, buf, pos):
        val_type = _buf_pos(buf, pos)
        if (
            val_type == BSER_INT8
            or val_type == BSER_INT16
            or val_type == BSER_INT32
            or val_type == BSER_INT64
        ):
            return self.unser_int(buf, pos)
        elif val_type == BSER_REAL:
            val = struct.unpack_from(b"=d", buf, pos + 1)[0]
            return (val, pos + 9)
        elif val_type == BSER_TRUE:
            return (True, pos + 1)
        elif val_type == BSER_FALSE:
            return (False, pos + 1)
        elif val_type == BSER_NULL:
            return (None, pos + 1)
        elif val_type == BSER_BYTESTRING:
            return self.unser_bytestring(buf, pos)
        elif val_type == BSER_UTF8STRING:
            return self.unser_utf8_string(buf, pos)
        elif val_type == BSER_ARRAY:
            return self.unser_array(buf, pos)
        elif val_type == BSER_OBJECT:
            return self.unser_object(buf, pos)
        elif val_type == BSER_TEMPLATE:
            return self.unser_template(buf, pos)
        else:
            raise ValueError(
                "unhandled bser opcode 0x%s"
                % binascii.hexlify(val_type).decode("ascii")
            )


def _pdu_info_helper(buf):
    bser_version = -1
    if buf[0:2] == EMPTY_HEADER[0:2]:
        bser_version = 1
        bser_capabilities = 0
        expected_len, pos2 = Bunser.unser_int(buf, 2)
    elif buf[0:2] == EMPTY_HEADER_V2[0:2]:
        if len(buf) < 8:
            raise ValueError("Invalid BSER header")
        bser_version = 2
        bser_capabilities = struct.unpack_from("I", buf, 2)[0]
        expected_len, pos2 = Bunser.unser_int(buf, 6)
    else:
        raise ValueError("Invalid BSER header")

    return bser_version, bser_capabilities, expected_len, pos2


def pdu_info(buf):
    info = _pdu_info_helper(buf)
    return info[0], info[1], info[2] + info[3]


def pdu_len(buf):
    info = _pdu_info_helper(buf)
    return info[2] + info[3]


def loads(buf, mutable: bool = True, value_encoding=None, value_errors=None):
    """Deserialize a BSER-encoded blob.

    @param buf: The buffer to deserialize.
    @type buf: bytes

    @param mutable: Whether to return mutable results.
    @type mutable: bool

    @param value_encoding: Optional codec to use to decode values. If
                           unspecified or None, return values as bytestrings.
    @type value_encoding: str

    @param value_errors: Optional error handler for codec. 'strict' by default.
                         The other most common argument is 'surrogateescape' on
                         Python 3. If value_encoding is None, this is ignored.
    @type value_errors: str
    """

    info = _pdu_info_helper(buf)
    expected_len = info[2]
    pos = info[3]

    if len(buf) != expected_len + pos:
        raise ValueError(
            "bser data len %d != header len %d" % (expected_len + pos, len(buf))
        )

    bunser = Bunser(
        mutable=mutable, value_encoding=value_encoding, value_errors=value_errors
    )

    return bunser.loads_recursive(buf, pos)[0]


def load(fp, mutable: bool = True, value_encoding=None, value_errors=None):
    from . import load

    return load.load(fp, mutable, value_encoding, value_errors)
