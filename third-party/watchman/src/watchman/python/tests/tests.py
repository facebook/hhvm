#!/usr/bin/env python
# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import binascii
import collections
import inspect
import os
import struct
import sys
import tempfile
import unittest
import uuid

from pywatchman import (
    bser,
    client,
    load,
    pybser,
    SocketConnectError,
    SocketTimeout,
    Transport,
    WatchmanError,
)


if os.path.basename(bser.__file__) == "pybser.py":
    raise Exception(
        "bser module resolved to pybser! Something is broken in your build. __file__={!r}, sys.path={!r}".format(
            bser.__file__, sys.path
        )
    )

PILE_OF_POO = "\U0001F4A9"
NON_UTF8_STRING = b"\xff\xff\xff"


class TestSocketTimeout(unittest.TestCase):
    def test_exception_handling(self):
        try:
            raise SocketTimeout("should not raise")
        except WatchmanError:
            pass


class TestTransportErrorHandling(unittest.TestCase):
    def test_transport_error(self):
        buf = '{"foo":"bar"}'
        failAfterBytesRead = 5

        class FakeFailingTransport(Transport):
            def __init__(self, sockpath, timeout):
                self.readBuf = buf
                self.readBufPos = 0
                self.writeBuf = []
                self.closed = False

            def close(self):
                self.closed = True

            def readBytes(self, size):
                readEnd = self.readBufPos + size
                if readEnd > failAfterBytesRead:
                    raise IOError(23, "fnord")
                elif readEnd > len(self.readBuf):
                    return ""
                read = self.readBuf[self.readBufPos : self.readBufPos + size]
                self.readBufPos += size
                return read

            def write(self, buf):
                self.writeBuf.extend(buf)

        c = client(
            sockpath="",
            transport=FakeFailingTransport,
            sendEncoding="json",
            recvEncoding="json",
        )
        try:
            c.query("foobarbaz")
            self.assertTrue(False, "expected a WatchmanError")
        except WatchmanError as e:
            self.assertIn(
                "I/O error communicating with watchman daemon: "
                + "errno=23 errmsg=fnord, while executing "
                + "('foobarbaz',)",
                str(e),
            )
        except Exception as e:
            self.assertTrue(False, "expected a WatchmanError, but got " + str(e))


class TestLocalTransport(unittest.TestCase):
    def test_missing_socket_file_raises_connect_error(self):
        socket_path = self.make_deleted_socket_path()
        c = client(sockpath=socket_path, transport="local")
        with self.assertRaises(SocketConnectError):
            with c:
                pass

    def make_deleted_socket_path(self):
        if os.name == "nt":
            path = self.make_deleted_windows_socket_path()
        else:
            path = self.make_deleted_unix_socket_path()
        self.assertFalse(os.path.exists(path))
        return path

    def make_deleted_windows_socket_path(self):
        return "\\\\.\\pipe\\pywatchman-test-{}".format(uuid.uuid1().hex)

    def make_deleted_unix_socket_path(self):
        temp_dir = tempfile.mkdtemp()
        return os.path.join(temp_dir, "socket")


def expand_bser_mods(test_class):
    """
    A decorator function used to create a class for bser and pybser
    variants of the test.
    """

    # We do some rather hacky things here to define new test class types
    # in our caller's scope.  This is needed so that the unittest TestLoader
    # will find the subclasses we define.
    caller_scope = inspect.currentframe().f_back.f_locals

    flavors = [(bser, "Bser"), (pybser, "PyBser")]
    for (mod, suffix) in flavors:

        def make_class(mod, suffix):
            subclass_name = test_class.__name__ + suffix

            # Define a new class that derives from the input class
            class MatrixTest(test_class):
                def init_bser_mod(self):
                    self.bser_mod = mod

            # Set the name and module information on our new subclass
            MatrixTest.__name__ = subclass_name
            MatrixTest.__qualname__ = subclass_name
            MatrixTest.__module__ = test_class.__module__

            caller_scope[subclass_name] = MatrixTest

        make_class(mod, suffix)


class FakeFile:
    def __init__(self, data):
        self._data = data
        self._ptr = 0

    def readinto(self, buf):
        l = len(buf)
        if len(self._data) - self._ptr < l:
            return None
        buf[:] = self._data[self._ptr : self._ptr + l]
        self._ptr += l
        return l


@expand_bser_mods
class TestBSERDump(unittest.TestCase):
    def setUp(self):
        self.init_bser_mod()

    def raw(self, structured_input, bser_output):
        enc = self.bser_mod.dumps(structured_input)
        self.assertEqual(enc, bser_output)

    def roundtrip(self, val, mutable=True, value_encoding=None, value_errors=None):
        enc = self.bser_mod.dumps(val)
        # print("# %s  -->  %s" % (repr(val),
        #                         binascii.hexlify(enc).decode('ascii')))
        dec = self.bser_mod.loads(
            enc, mutable, value_encoding=value_encoding, value_errors=value_errors
        )
        self.assertEqual(val, dec)

        fp = FakeFile(enc)
        dec = self.bser_mod.load(
            fp, mutable, value_encoding=value_encoding, value_errors=value_errors
        )
        self.assertEqual(val, dec)

    def munged(self, val, munged, value_encoding=None, value_errors=None):
        enc = self.bser_mod.dumps(val)
        # print("# %s  -->  %s" % (repr(val),
        #                         binascii.hexlify(enc).decode('ascii')))
        dec = self.bser_mod.loads(
            enc, value_encoding=value_encoding, value_errors=value_errors
        )
        self.assertEqual(munged, dec)

    def test_raw(self):
        self.raw(
            {"name": "Tom"},
            b"\x00\x01\x05\x10\x00\x00\x00\x01\x03\x01\x02\x03\x04name\x02"
            b"\x03\x03Tom",
        )
        self.raw(
            {"names": ["Tom", "Jerry"]},
            b"\x00\x01\x05\x1c\x00\x00\x00\x01\x03\x01\x02\x03\x05names\x00"
            b"\x03\x02\x02\x03\x03Tom\x02\x03\x05Jerry",
        )
        self.raw(
            ["Tom", "Jerry"],
            b"\x00\x01\x05\x11\x00\x00\x00\x00\x03\x02\x02\x03\x03Tom\x02"
            b"\x03\x05Jerry",
        )
        self.raw(
            [1, 123, 12345, 1234567, 12345678912345678],
            b"\x00\x01\x05\x18\x00\x00\x00\x00\x03\x05\x03\x01\x03{\x0490"
            b"\x05\x87\xd6\x12\x00\x06N\xd6\x14^T\xdc+\x00",
        )

    def test_int(self):
        self.roundtrip(1)
        self.roundtrip(0x100)
        self.roundtrip(0x10000)
        self.roundtrip(0x10000000)
        self.roundtrip(0x1000000000)

    def test_negative_int(self):
        self.roundtrip(-0x80)
        self.roundtrip(-0x8000)
        self.roundtrip(-0x80000000)
        self.roundtrip(-0x8000000000000000)

    def test_float(self):
        self.roundtrip(1.5)

    def test_bool(self):
        self.roundtrip(True)
        self.roundtrip(False)

    def test_none(self):
        self.roundtrip(None)

    def test_string(self):
        self.roundtrip(b"hello")

        # For Python 3, here we can only check that a Unicode string goes in,
        # not that a Unicode string comes out.
        self.munged("Hello", b"Hello")

        self.roundtrip("Hello", value_encoding="utf8")
        self.roundtrip("Hello", value_encoding="ascii")
        self.roundtrip("Hello" + PILE_OF_POO, value_encoding="utf8")

        # can't use the with form here because Python 2.6
        self.assertRaises(
            UnicodeDecodeError,
            self.roundtrip,
            "Hello" + PILE_OF_POO,
            value_encoding="ascii",
        )
        self.munged(
            "Hello" + PILE_OF_POO,
            "Hello",
            value_encoding="ascii",
            value_errors="ignore",
        )
        self.roundtrip(b"hello" + NON_UTF8_STRING)
        self.assertRaises(
            UnicodeDecodeError,
            self.roundtrip,
            b"hello" + NON_UTF8_STRING,
            value_encoding="utf8",
        )
        self.munged(
            b"hello" + NON_UTF8_STRING,
            "hello",
            value_encoding="utf8",
            value_errors="ignore",
        )
        # TODO: test non-UTF8 strings with surrogateescape in Python 3

        ustr = "\xe4\xf6\xfc"
        self.munged(ustr, ustr.encode("utf-8"))

    def test_list(self):
        self.roundtrip([1, 2, 3])
        self.roundtrip([1, b"helo", 2.5, False, None, True, 3])

    def test_tuple(self):
        self.munged((1, 2, 3), [1, 2, 3])
        self.roundtrip((1, 2, 3), mutable=False)

    def test_dict(self):
        self.roundtrip({"hello": b"there"})
        self.roundtrip({"hello": "there"}, value_encoding="utf8")
        self.roundtrip({"hello": "there"}, value_encoding="ascii")
        self.roundtrip({"hello": "there" + PILE_OF_POO}, value_encoding="utf8")

        # can't use the with form here because Python 2.6
        self.assertRaises(
            UnicodeDecodeError,
            self.roundtrip,
            {"hello": "there" + PILE_OF_POO},
            value_encoding="ascii",
        )
        self.munged(
            {"Hello": "there" + PILE_OF_POO},
            {"Hello": "there"},
            value_encoding="ascii",
            value_errors="ignore",
        )
        self.roundtrip({"Hello": b"there" + NON_UTF8_STRING})
        self.assertRaises(
            UnicodeDecodeError,
            self.roundtrip,
            {"hello": b"there" + NON_UTF8_STRING},
            value_encoding="utf8",
        )
        self.munged(
            {"Hello": b"there" + NON_UTF8_STRING},
            {"Hello": "there"},
            value_encoding="utf8",
            value_errors="ignore",
        )

        obj = self.bser_mod.loads(self.bser_mod.dumps({"hello": b"there"}), False)
        self.assertEqual(1, len(obj))
        self.assertEqual(b"there", obj.hello)
        self.assertEqual(b"there", obj["hello"])
        self.assertEqual(b"there", obj[0])
        # make sure this doesn't crash
        self.assertRaises(Exception, lambda: obj[45.25])
        (hello,) = obj  # sequence/list assignment
        self.assertEqual(b"there", hello)

    def assertItemAttributes(self, dictish, attrish):
        self.assertEqual(len(dictish), len(attrish))
        # Use items for compatibility across Python 2 and 3.
        for k, v in dictish.items():
            self.assertEqual(v, getattr(attrish, k))

    def test_template(self):
        # since we can't generate the template bser output, here's a
        # a blob from the C test suite in watchman
        templ = (
            b"\x00\x01\x03\x28"
            + b"\x0b\x00\x03\x02\x02\x03\x04\x6e\x61\x6d\x65\x02"
            + b"\x03\x03\x61\x67\x65\x03\x03\x02\x03\x04\x66\x72"
            + b"\x65\x64\x03\x14\x02\x03\x04\x70\x65\x74\x65\x03"
            + b"\x1e\x0c\x03\x19"
        )
        dec = self.bser_mod.loads(templ)
        exp = [
            {"name": b"fred", "age": 20},
            {"name": b"pete", "age": 30},
            {"name": None, "age": 25},
        ]
        self.assertEqual(exp, dec)
        res = self.bser_mod.loads(templ, False)

        for i in range(0, len(exp)):
            self.assertItemAttributes(exp[i], res[i])

    def test_pdu_info(self):
        enc = self.bser_mod.dumps(1)
        DEFAULT_BSER_VERSION = 1
        DEFAULT_BSER_CAPABILITIES = 0
        self.assertEqual(
            (DEFAULT_BSER_VERSION, DEFAULT_BSER_CAPABILITIES, len(enc)),
            self.bser_mod.pdu_info(enc),
        )

        # try a bigger one; prove that we get the correct length
        # even though we receive just a portion of the complete
        # data
        enc = self.bser_mod.dumps([1, 2, 3, "hello there, much larger"])
        self.assertEqual(
            (DEFAULT_BSER_VERSION, DEFAULT_BSER_CAPABILITIES, len(enc)),
            self.bser_mod.pdu_info(enc[0:7]),
        )

    def test_pdu_len(self):
        enc = self.bser_mod.dumps(1)
        self.assertEqual(len(enc), self.bser_mod.pdu_len(enc))

        # try a bigger one; prove that we get the correct length
        # even though we receive just a portion of the complete
        # data
        enc = self.bser_mod.dumps([1, 2, 3, "hello there, much larger"])
        self.assertEqual(len(enc), self.bser_mod.pdu_len(enc[0:7]))

    def test_garbage(self):
        # can't use the with form here because Python 2.6
        self.assertRaises(ValueError, self.bser_mod.loads, b"\x00\x01\n")
        self.assertRaises(ValueError, self.bser_mod.loads, b"\x00\x01\x04\x01\x00\x02")
        self.assertRaises(ValueError, self.bser_mod.loads, b"\x00\x01\x07")
        self.assertRaises(ValueError, self.bser_mod.loads, b"\x00\x01\x03\x01\xff")

        self.assertRaises(ValueError, self.bser_mod.pdu_info, b"\x00\x02")

    def test_string_lengths(self):
        self.roundtrip(b"")

        self.roundtrip(b"a" * ((1 << 8) - 2))
        self.roundtrip(b"a" * ((1 << 8) - 1))
        self.roundtrip(b"a" * ((1 << 8) + 0))
        self.roundtrip(b"a" * ((1 << 8) + 1))

        self.roundtrip(b"a" * ((1 << 16) - 2))
        self.roundtrip(b"a" * ((1 << 16) - 1))
        self.roundtrip(b"a" * ((1 << 16) + 0))
        self.roundtrip(b"a" * ((1 << 16) + 1))

        self.roundtrip(b"a" * ((1 << 24) - 2))
        self.roundtrip(b"a" * ((1 << 24) - 1))
        self.roundtrip(b"a" * ((1 << 24) + 0))
        self.roundtrip(b"a" * ((1 << 24) + 1))

    def test_big_string_lengths(self):
        # These tests take a couple dozen seconds with the Python
        # implementation of bser. Keep them in a separate test so we can run
        # them conditionally.
        self.assertRaises(MemoryError, self.bser_mod.dumps, b"a" * ((1 << 32) - 2))
        self.assertRaises(MemoryError, self.bser_mod.dumps, b"a" * ((1 << 32) - 1))
        self.assertRaises(MemoryError, self.bser_mod.dumps, b"a" * ((1 << 32) + 0))
        self.assertRaises(MemoryError, self.bser_mod.dumps, b"a" * ((1 << 32) + 1))

    def test_fuzz_examples(self):
        def t(ex: bytes):
            try:
                document = b"\x00\x01\x05" + struct.pack("@i", len(ex)) + ex
                print("encoded", document)
                self.bser_mod.loads(document)
            except Exception:
                # Exceptions are okay - abort is not.
                pass

        t(b"\x03\x00")
        t(b"\x02")
        t(b"\x07")


if __name__ == "__main__":
    unittest.main()
