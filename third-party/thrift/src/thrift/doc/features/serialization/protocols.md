# Serialization Protocols

A serialization protocol in Thrift is a format that defines how data is serialized into a sequence of bytes and deserialized from it.

## Thrift Types

There are 13 different Thrift types that can appear in the serialized output.

| Type   | Description                                                                                          |
| ---    | ---                                                                                                  |
| STOP   | STOP does not represent data. It is used to indicate that no fields are left in a serialized struct. |
| BOOL   | Boolean                                                                                              |
| BYTE   | 8-bit signed integer                                                                                 |
| DOUBLE | Double precision floating point number                                                               |
| I16    | 16-bit signed integer                                                                                |
| I32    | 32-bit signed integer or a Thrift enum                                                               |
| I64    | 64-bit signed integer                                                                                |
| STRING | A String                                                                                             |
| STRUCT | A Thrift struct or union                                                                             |
| MAP    | Key-value mapping                                                                                    |
| LIST   | List of Thrift values                                                                                |
| SET    | Set of Thrift values                                                                                 |
| FLOAT  | Single precision floating point number                                                               |

## Serialization Formats

<!-- Protocol names should be capitalized, e.g. Compact, Binary, Frozen. -->

Each serialization format encodes the above Thrift types in a different way.

### Binary Protocol

The Binary protocol produces data in a non-human-readable format.

#### TYPE CODES

| Type   | Code   |
| ---    | ---    |
| STOP   | `0x00` |
| BOOL   | `0x02` |
| BYTE   | `0x03` |
| DOUBLE | `0x04` |
| I16    | `0x06` |
| I32    | `0x08` |
| I64    | `0x0A` |
| STRING | `0x0B` |
| STRUCT | `0x0C` |
| MAP    | `0x0D` |
| SET    | `0x0E` |
| LIST   | `0x0F` |
| FLOAT  | `0x13` |

#### STRUCTS

A struct is serialized as a sequence of fields. Each field consists of an 8-bit type code (from the table above), a 16-bit field ID, and then the serialized value. The size of the serialized value depends on the value type. At the end, a STOP (`0x00`) is written; there is no field id following the STOP code.

| Type Code | Field ID | Value    |
| ---       | ---      | ---      |
| 8 Bits    | 16 Bits  | Variable |

Unions are serialized in the same way, but instead of having a sequence of fields, they contain exactly one field.

#### BOOL

Booleans are serialized as 8-bit values where `0x00` is False and `0x01` is True

#### NUMERIC TYPES

The Thrift numeric types are `byte` (`i8`), `i16`, `i32`, `i64`, `float`, and `double`. These are all serialized using their big-endian n-bit binary representations.

#### STRING

Strings are encoded using a 32-bit length followed by a sequence of 8-bit characters

| Length | Payload        |
| ---    | ---            |
| 32-bit | Length * 8-bit |

#### LIST/SET

Lists and sets are serialized as follows. First, the element type code is written, then the signed 32-bit length, then the elements. Negative length values are invalid.

| Type Code | Length | Payload           |
| ---       | ---    | ---               |
| 8-bit     | 32-bit | Length * Variable |

#### MAP

Maps are serialized using the 8-bit type codes of the key and value types followed by the length and payload. The payload contains the key value pairs. Length is signed 32-bit; negative values are invalid.

| Key Type Code | Value Type Code | Length | Key 1    | Value 1  | ... | Key n    | Value n  |
| ---           | ---             | ---    | ---      | ---      | --- | ---      | ---      |
| 8-bit         | 8-bit           | 32-bit | Variable | Variable |     | Variable | Variable |

### Compact Protocol

The Compact protocol is similar to the Binary protocol, but some values are encoded so as to use fewer bytes. In particular, integral types are encoded using the [varint](https://developers.google.com/protocol-buffers/docs/encoding) format.

#### TYPE CODES

| Type         | Code  |
| ---          | ---   |
| STOP         | `0x0` |
| BOOL (True)  | `0x1` |
| BOOL (False) | `0x2` |
| BYTE         | `0x3` |
| I16          | `0x4` |
| I32          | `0x5` |
| I64          | `0x6` |
| DOUBLE       | `0x7` |
| STRING       | `0x8` |
| LIST         | `0x9` |
| SET          | `0xA` |
| MAP          | `0xB` |
| STRUCT       | `0xC` |
| FLOAT        | `0xD` |

#### STRUCTS

A struct is serialized as a sequence of fields.
Each field consists of a 4-bit field offset and a 4-bit type code. If the field offset is non-zero then it is added to the previous field ID that was written to obtain the current field ID (starting at 0). If it is 0, then the field ID is encoded as a variable length 16-bit integer. Boolean fields are encoded in the type code.
Type code (from the table above), a 16-bit field ID, and then the serialized value. The size of the serialized value depends on the value type. At the end, STOP (`0x00`) is written; there is no additional field ID after the STOP code, even if the field offset is 0.

| Field Offset | Type Code | Value    |
| ---          | ---       | ---      |
| 4 Bits       | 4 Bits    | Variable |

OR

| Field Offset | Type Code | Field ID | Value    |
| ---          | ---       | ---      | ---      |
| `0x0`        | 4 Bits    | Variable | Variable |

Unions are serialized in the same way, but instead of having a sequence of fields, they contain exactly one field.

#### BOOL

Booleans that are field values are encoded in the type code, so they have no additional serialization. If the boolean is part of collection (list, map, or set), then it is encoded as an 8-bit value where `0x02` is False and `0x01` is True.

#### INTEGRAL TYPES LARGER THAN 1 BYTE

The integral types that are larger than 1 Byte (`i16`, `i32`, and `i64`) are encoded using the varint format.

#### OTHER NUMERIC TYPE

The remaining Thrift numeric types (`byte`, `float`, and `double`) are serialized in the same way as in the Binary protocol.

#### STRING

Strings are encoded using a variable-sized length followed by a sequence of 8-bit characters

| Length   | Payload        |
| ---      | ---            |
| Variable | Length * 8-bit |

#### LIST/SET

Lists and sets are serialized as follows. If the length is less than 15, then the first byte contains the length and the element type code. If the length is 15 or more, then the first byte contain `0xF` and the element type code followed by the length serialized as a varint-encoded 32-bit integer. Serialized elements come next.

| Length | Type Code | Payload           |
| ---    | ---       | ---               |
| 4-bit  | 4-bit     | Length * Variable |

OR

| Filler | Type Code | Length   | Payload           |
| ---    | ---       | ---      | ---               |
| `0xF`  | 4-bit     | Variable | Length * Variable |

#### MAP

Maps contain a variable encoded length followed by the key and value types encoded in a single byte. Next comes the payload which is encoded in a similar way to the Binary protocol.

| Length   | Key Type | Value Type | Key 1    | Value 1  | ... | Key n    | Value n  |
| ---      | ---      | ---        | ---      | ---      | --- | ---      | ---      |
| Variable | 4-bit    | 4-bit      | Variable | Variable |     | Variable | Variable |

#### Varint Encoding

Varint encoding is a variable length encoding for integer values.
Unsigned integers are simplest. They are divided into 7-bit groups, from least to most significant, until there are no more non-zero groups. Each group is encoded as `1xxxxxxx` until the last group which is `0xxxxxxx`. So `0x12` - `00010010` in binary - would be literally encoded that way, since it's only a single group. `0x37f0` is `00_1111101_1110000` in binary (using `_` as a visual separator to show the 7 bit groups), so would be encoded as `1_1110000` `0_1111101` (again, `_` as visual separator).
Signed integers could be treated the same as unsigned, but it would result in all negative values being treated as large positive numbers, with correspondingly large encodings. It would be better to encode numbers with small absolute values more compactly, regardless of sign. Varint does this via "zigzag" encoding, where all numbers are mapped to a positive value before encoding:

| Original | Encoded as |
| ---      | ---        |
| 0        | 0          |
| -1       | 1          |
| 1        | 2          |
| -2       | 3          |
| 2        | 4          |

Or more generally: `(x << 1) ^ (x >>> (bits-1))` where `bits` is the size of the value in bits, and `>>>` is an arithmetic (sign-extending) shift.
Reversing this is `(x >> 1) ^ -(x & 1)`.

### Handling Unknown Fields

If a decoder encounters an unknown field id while decoding a struct/union, it can continue by skipping that value and resuming with the rest of the stream. To allow this, fields and containers (map, set, list) contain the type code of the unknown type.
For non-compound types, skipping the value is trivial.
For compound types (struct/union and containers), skipping the value is a matter of skipping each field/entry; if a field/entry is also compound then the process is recursive.

## Other Protocols

### Frozen2

<!-- https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/frozen2/ -->

Frozen2 serializes data into a contiguous chunk of buffer, using its associated *layout*. *Layout* files are autogenerated from Thrift struct definition, and describes to the library what fields the struct contains.

A contiguous buffer means several advantages:
* For offline use (serializing Thrift struct to disk, etc.), `mmap`able data.
* To read values from buffer after serialization, location of the value is already known (think of a C struct), so values can be extracted in-place. Using specialized API is sometimes required.

### Debug Protocol

This protocol prints the Thrift object as a human readable, nicely indented string. It is write only now, you cannot deserialize from such a string. There is no guarantee that the format won't change -- it might be evolved in a non-backward compatible way. It should only be used for logging.

## Deprecated

- JSON: This protocol serializes Thrift objects into JSON objects.
- SimpleJSON: This protocol also serializes to JSON but doesn't output verbose field types and uses field names instead of IDs (which affects schema evolution).
* PHPSerialize: This protocol serializes Thrift objects into PHP's "serialize" format. It is write-only now; you cannot deserialize from PHP.
