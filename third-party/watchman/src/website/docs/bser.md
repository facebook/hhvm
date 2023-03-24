---
title: BSER Binary Protocol
section: Internals
---

The basic JSON protocol in watchman allows quick and easy integration.
Applications with higher performance requirements may want to consider the
binary protocol instead.

The binary protocol is enabled by the client sending the byte sequence
"\x00x\x01".

## PDU

A PDU is prefixed by its length expressed as an encoded integer. This allows the
peer to determine how much storage is required to read and decode it.

## Arrays

Arrays are indicated by a `0x00` byte value followed by an integer value to
indicate how many items follow. Then each item is encoded one after the other.

## Objects

Objects are indicated by a `0x01` byte value followed by an integer value to
indicate the number of properties in the object. Then each key/value pair is
encoded one after the other.

## Strings

Strings are indicated by a `0x02` byte value followed by an integer value to
indicate the number of bytes in the string, followed by the bytes of the string.

### Encoding

Unlike JSON, strings are not defined as having any particular encoding; they are
transmitted as binary strings. This is because the underlying filesystem APIs
don't define any particular encoding for names.

_Exception:_ Keys in objects that are defined by watchman commands are always
ASCII. In general, keys in objects are always UTF-8.

_Rationale:_ Several programming languages like Python 3 expect all text to be
in a particular encoding and make it inconvenient to pass in bytestrings or
other encodings. Also, the primary purpose of not defining an encoding is that
filenames don't always have one, and filenames are unlikely to show up as keys.

## Integers

All integers are signed and transmitted in the host byte order of the system
running the watchman daemon.

- `0x03` indicates an int8_t. It is followed by the int8_t value.
- `0x04` indicates an int16_t. It is followed by the int16_t value.
- `0x05` indicates an int32_t. It is followed by the int32_t value.
- `0x06` indicates an int64_t. It is followed by the int64_t value.

## Real

A real number is indicated by a `0x07` byte followed by 8 bytes of double value.

## Boolean

- `0x08` indicates boolean true
- `0x09` indicates boolean false

## Null

`0x0a` indicates the null value

## Array of Templated Objects

`0x0b` indicates a compact array of objects follows. Some of the bigger
datastructures returned by watchman are tabular data expressed as an array of
objects. This serialization type factors out the repeated object keys into a
header array listing the keys, followed by an array containing all the values of
the objects.

To represent missing keys in templated arrays, the `0x0c` encoding value may be
present. If encountered it is interpreted as meaning that there is no value for
the key that would have been decoded in this position. This is distinct from the
null value.

For example:

```json
[
   {"name": "fred", "age": 20},
   {"name": "pete", "age": 30},
   {"age": 25 },
]
```

is represented similar to:

```json
["name", "age"],
[
  "fred", 20,
  "pete", 30,
  0x0c,   25
]
```

The precise sequence is:

```
0b          template
00          array     -- start prop names
0302        int, 2    -- two prop names
02          string    -- first prop "name"
0304        int, 4
6e616d65    "name"
02          string    -- 2nd prop "age"
0303        int, 3
616765      "age"
0303        int, 3    -- there are 3 objects
02          string    -- object 1, prop 1 name=fred
0304        int, 4
66726564    "fred"
0314        int 0x14  -- object 1, prop 2 age=20
02          string    -- object 2, prop 1 name=pete
0304        int 4
70657465    "pete"
031e        int, 0x1e -- object 2, prop 2 age=30
0c          skip      -- object 3, prop 1, not set
0319        int, 0x19 -- object 3, prop 2 age=25
```

Note: to avoid hostile "decompression bombs", Watchman will reject parsing
template objects that have an empty set of keys.
