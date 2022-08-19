============
CBOR binding
============

Overview
========

C functions to encode/decode values in CBOR format, and a simple command
line utility to convert between JSON and CBOR.

To integrate CBOR into your application:

* Call ``duk_cbor_encode()`` and ``duk_cbor_decode()`` directly if a C API
  is enough.

* Call ``duk_cbor_init()`` to register a global ``CBOR`` object with
  ECMAScript bindings ``CBOR.encode()`` and ``CBOR.decode()``, roughly
  matching https://github.com/paroga/cbor-js.

Basic usage of the ``jsoncbor`` conversion tool::

    $ make jsoncbor
    [...]
    $ cat test.json | ./jsoncbor -e   # writes CBOR to stdout
    $ cat test.cbor | ./jsoncbor -d   # writes JSON to stdout

CBOR objects are decoded into ECMAScript objects, with non-string keys
coerced into strings.

Direct support for CBOR is likely to be included in the Duktape API in the
future.  This extra will then become unnecessary.

CBOR
====

CBOR is a standard format for JSON-like binary interchange.  It is
faster and smaller, and can encode more data types than JSON.  In particular,
binary data can be serialized without encoding e.g. in base-64.  These
properties make it useful for e.g. storing state files, IPC, etc.

Some CBOR shortcomings for preserving information:

- No property attribute or inheritance support.
- No DAGs or looped graphs.
- Array objects with properties lose their non-index properties.
- Array objects with gaps lose their gaps as they read back as undefined.
- Buffer objects and views lose much of their detail besides the raw data.
- ECMAScript strings cannot be fully represented; strings must be UTF-8.
- Functions and native objects lose most of their detail.
- CBOR tags are useful to provide soft decoding information, but the tags
  are just integers from an IANA controlled space with no space for custom
  tags.  So tags cannot be easily used for private, application specific tags.

Future work
===========

- Decode test cases, also for cases we don't produce.
- https://datatracker.ietf.org/doc/draft-jroatch-cbor-tags/?include_text=1
  could be used for typed arrays.
- 64-bit integer encoding.
- Better 64-bit integer decoding (workaround for non-64-bit targets).
- Objects with non-string keys, could be represented as a Map.
- Definite-length object encoding even when object has more than 23 keys.
