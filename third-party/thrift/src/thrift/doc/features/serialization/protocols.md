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

The integral types that are larger than 1 Byte (`i16`, `i32`, and `i64`) are encoded using the little-endian varint format.

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
Unsigned integers are simplest. They are divided into 7-bit groups, from least to most significant, until there are no more non-zero groups. Each group is encoded as `1xxxxxxx` until the last group which is `0xxxxxxx`. So `0x12` - `00010010` in binary - would be literally encoded that way, since it's only a single group. `0x3ef0` is `00_1111101_1110000` in binary (using `_` as a visual separator to show the 7 bit groups), so would be encoded as `1_1110000` `0_1111101` (again, `_` as visual separator).
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

### JSON5 Protocol

#### Conventions

* **Encoding:** Input must be UTF-8 encoded; behavior is undefined for non-UTF-8 input.
* **Format:** Input must be valid JSON5 data; invalid input is rejected.
* **Whitespace:** Leading/trailing whitespace are accepted; JSON5 comments (`//`, `/* ... */`) allowed anywhere whitespace is permitted (per JSON5 spec). UTF-8 BOM (byte sequence `EF BB BF` at start of input) is rejected.
* **Case sensitivity:** All matching is case-sensitive (booleans, enum names, field names).
* **Null handling:** Struct/union field `null` → treated as absent (a field with value `null` and an absent field are equivalent after deserialization); `null` elsewhere is treated as type mismatch.
* **Type mismatch:** Rejected (e.g., `true` for integer field). Note: This differs from Binary/Compact protocols, where type-mismatched fields are skipped silently. Since JSON5 input can be created by hand, silently skipping type-mismatched data is error-prone.
* **Rejection:** Causes a deserialization exception; no partial values returned.

#### BOOL

**Serialization**: `true` or `false` as JSON literal.

**Deserialization**: Accepts `true`, `false`, `"true"`, `"false"` (case-sensitive). Rejects numeric values (`0`, `1`), case variants (`TRUE`), alternative strings (`"yes"`).

#### INTEGRAL (byte, i16, i32, i64)

**Serialization**: Decimal integer literal.

**Deserialization**: Accepts

* Any valid JSON5 integer, which includes
  * Decimal integer literals.
  * Leading `+`.
  * Hex literals (`0x` prefix; prefix and digits are both case-insensitive, e.g., `0x2a`, `0X2A`, `0x2A` all accepted).
    * Note: Hex literals may have leading zeros after the prefix, e.g., `0x007` is valid.
  * Signed hex: `-0x2A` yields \-42; `+0x2A` yields 42\.
* Quoted numeric strings (same rules as above, e.g., `"0x2A"`).

| Input | Result |
| :---- | :---- |
| `42`, `+42`, `"42"`, `0x2A`, `0X2a`, `"0x2A"` | 42 |

Rejects:

* Overflow (e.g., `128` for byte, `32768` for i16).
* Non-integers (e.g., `0.0`, `Infinity`, `"abc"`, `""`).
* Exponent notation (e.g., `1e3`, `"1e3"`) (rejected to avoid ambiguity with floating-point values).
* Leading zeros in decimal literals (any leading zero except bare `0` itself, e.g., `007`, `"007"`, `00`, `01` are rejected, whether bare or quoted).
* Binary/octal literals (e.g., `0b101010`, `0o10`).

#### FLOAT/DOUBLE

**Serialization**:

* Finite values: shortest round-trip decimal representation.
* `-0.0`: always written as `-0.0` (with explicit decimal point, not `-0`).
* Special values:

| Value | JSON output | JSON5 output |
| :---- | :---- | :---- |
| NaN | `"NaN"` | `NaN` |
| \-NaN | `"-NaN"` | `-NaN` |
| Infinity | `"Infinity"` | `Infinity` |
| \-Infinity | `"-Infinity"` | `-Infinity` |

**Deserialization**: Accepts

* JSON5 numeric literals.
* Special float literals.
* Integers if and only if converting to the target IEEE 754 type and back to integer yields the original value (bounds: \[-2²⁴, 2²⁴\] (`±16,777,216`) inclusive for float, \[-2⁵³, 2⁵³\] (`±9,007,199,254,740,992`) inclusive for double).
* Quoted numeric strings (same rules as above, e.g., `"3.14"`).

Rejects:

* Non-numeric strings (e.g., `"abc"`, `""`).
* Integers with precision loss (e.g., `11122233344455566`).

#### STRING

**Serialization**: JSON double-quoted strings with standard escape sequences (`\"`, `\\`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t`, `\uXXXX`).

**Deserialization**: Accepts any valid JSON5 strings. For example:

| Input | Result |
| :---- | :---- |
| "hello, world" | hello, world |
| 'hello, world' (single-quoted) | hello, world |
| "hello, \\<br/> world" (line continuation) | hello, world |
| "hello, \\nworld" | hello, <br/>world |
| "\\u0041" | A |

Rejects any invalid JSON5 strings.

#### BINARY

**Serialization**: JSON object with one encoding key:

| Key | Used When | Example |
| :---- | :---- | :---- |
| `utf-8` | The binary data is printable; see criteria below; | `{"utf-8": "hello"}` |
| `base64url` ([RFC 4648 §5](https://datatracker.ietf.org/doc/html/rfc4648#section-5)) | Otherwise | `{"base64url": "3q2-7w"}` |

Binary data is considered printable if and only if all of the following are true:

1. Bytes form valid UTF-8.
2. No C0 control characters (U+0000–U+001F) except `\b`, `\f`, `\n`, `\r`, `\t` which have JSON escape sequences.
3. No C1 control characters (U+0080–U+009F).

**Deserialization**: Accepts `utf-8`, `base64`, and `base64url` encoding keys (with or without padding).

For bare strings: if the string contains `-` or `_`, it is treated as base64url; if it contains `+` or `/`, it is treated as standard base64; if it contains neither (i.e., only characters common to both alphabets), it is decoded as standard base64.

For example:

| Input | Result |
| :---- | :---- |
| `{"utf-8": "hello"}` | "hello" |
| `{"base64url": "3q2-7w"}` | 0xDEADBEEF (base64url decoded) |
| `{"base64": "3q2+7w=="}` | 0xDEADBEEF (standard base64 decoded accepted, never emitted) |
| `"3q2-7w"` | 0xDEADBEEF |

Rejects:

* Invalid base64 as bare strings (e.g., `"Not Valid!"`).
* Base64 with invalid padding (e.g., `"3q2+7w==="`).
* Unsupported encoding keys (e.g., `{"invalid": "3q2-7w"}`).
* Multiple encoding keys (e.g., `{"utf-8": "hi", "base64": "aGk="}`).

:::caution

Bare strings that happen to be valid base64 are base64-decoded, not UTF-8. Example: `"aGk="` → bytes `hi`, not four characters. Use `{"utf-8": "..."}` to avoid ambiguity.

:::

#### ENUM

**Serialization**: `"NAME (value)"` (e.g., `"ONE (1)"`). Unknown values: `"(value)"` (e.g., `"(42)"`).

**Deserialization**: Accepts

* `"enum-name"` (e.g., `"ONE"`).
* `"enum-name (enum-value)"` (e.g., `"ONE (1)"`).
* `"(enum-value)"` (e.g., `"(1)"`).
* `enum-value` (e.g., `1`, `"1"`).

Note that for enum-value, we use the same rules as integral types, thus `"ONE (0x1)"` is also supported. Unknown integer values are accepted (consistent with Binary/Compact).

Rejects:

* Name-value mismatch (e.g., `"ONE (2)"`).
  * Note that if both `enum-name` and `enum-value` don't exist in the local schema, it's accepted since it's not considered a mismatch.
* Bare unknown name (e.g., `"VALUE_NOT_IN_SCHEMA"`).
* Floats (e.g., `"ONE (1.0)"`).
* I32 overflow (e.g., `2147483648`).

#### LIST/SET

**Serialization**: JSON arrays (with trailing commas per Output Formatting). List order preserved. Set order: [stable ascending](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/object-model-draft/#operation-isstablelessthan).

**Deserialization**: Accepts JSON arrays. Elements inside the array are decoded recursively. List order preserved. Set order not significant.

Rejects:

* Null elements.
* Type mismatch (e.g., quoted `"[1,2]"` is rejected since it's a JSON string, not a JSON array).
* Set duplicates (detected after full deserialization of all elements to their target type, e.g., `["1", "0x1"]` for `set<i32>`).

#### MAP

**Serialization**: Format depends on key type:

| Key Type | Format | Example |
| :---- | :---- | :---- |
| String or enum | JSON Object `{"my_key": "my_value"}`. | `{"ONE (1)": 1}` (for `map<Enum, i32>`) |
| All others | Array of `{"key": ..., "value": ...}` objects | `[{"key": "3q2-7w", "value": 1}]` (for `map<Binary, i32>`) |

Order: [stable ascending](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/object-model-draft/#operation-isstablelessthan).
**Deserialization**:

* The decoder accepts both Object and Array forms for compatibility:
  * if the JSON value is an object, object form is used;
  * if an array, array form is used.
* Object-form keys are decoded using map key type's full rules (e.g., for `map<Enum, V>`, both `"ONE (1)"` and `"1"` are accepted; see ENUM section above)
* Keys and values inside the Object or Array are decoded recursively.

For example:

| Input | Notes |
| :---- | :---- |
| `{"ONE (1)": 1}`| Object form, `map<Enum, i32>` |
| `[{"key": "ONE (1)", "value": 1}]`| Array form, `map<Enum, i32>` |
| `{"3q2-7w": 1}` | Object form, `map<binary, i32>` |
| `[{"key": "3q2-7w", "value": 1}]` | Array form, `map<binary, i32>` |
| `[{"key": [1, 2], "value": 3}]` | Array form, `map<list<i32>, i32>` |

:::caution

In `map<binary, V>`, object keys are base64-decoded as bytes. Use array form with `{"utf-8": "..."}` to avoid ambiguity.

:::

Rejects:

* Missing/extra fields in array entries. e.g., all the following data are rejected
  * `[{"key": 1}]`
  * `[{"key": 1, "value": 2, "extra": 3}]`
* Duplicate keys (after decoding, e.g., `{"1": 10, "0x1": 20}` for `map<i32, i32>`).
* Object form, but map's key type can not decode from a string (e.g., `{"[1,2]": 3}` for `map<list<i32>, i32>` -- `"[1,2]"` is rejected for `list<i32>`; see LIST section above).
* Null keys/values (e.g., `{"1": null}`).

#### STRUCT/UNION

**Serialization**: Written as JSON objects with field names as keys, in ascending field ID order. Unions have at most one field.
**Deserialization**: Accepts a JSON object with the following key formats

* `"field-name"` (e.g., `"myField"`)
* `"field-name (field-id)"` (e.g., `"myField (30)"`)
* `"(field-id)"` (e.g., `"(30)"`)
* `"field-id"` (e.g., `"30"`)

Note that for field-id, we use the same rules as integral types, thus `"myField (0x1E)"` is also supported.

Behavior:

* Unknown fields: silently skipped (This is important for forward compatibility).
* Null field values: silently skipped.
* Unions: multiple fields → rejected; empty `{}` → unset (valid).
* Duplicate keys: rejected (includes keys resolving to same field ID).

For example:

| Input | Result |
| :---- | :---- |
| `{"myField": 1}` | Matched by field name |
| `{"myField (30)": 1}` | Matched by name and field ID |
| `{"(30)": 1}` | Matched by field ID only |
| `{"30": 1}` | Matched by field ID (bare integer string) |
| `{"field_1": null, "field_2": 2}` | Identical to parsing `{"field_2": 2}` |
| `{"unknown_field (42)": [1,2,3]}` | Unknown fields are skipped |

Rejects:

* Name/ID conflicts (e.g., `"fieldA (30)"` where fieldA actually has ID 20).
* Duplicate keys.
* Top-level null.
* Field ID that overflows i16 (e.g., `"field (32768)"`).

#### Output Formatting

Serialization output uses 2-space indentation with the following rules:

* Newlines after opening / before closing brackets, and between elements.
* Comma + newline between entries.
* Colon + single space between keys and values.
* Empty containers (`[]`, `{}`) on single line.

By default output is standard JSON ([RFC 8259](https://datatracker.ietf.org/doc/html/rfc8259)); the serializer should provide an option to output [JSON5](https://spec.json5.org/) with the following changes:

* Trailing comma after last entry.
* Unquoted object keys when valid JSON5 IdentifierNames.
* Special floating values should be unquoted (see FLOAT/DOUBLE section above).

### Handling Unknown Fields

If a decoder encounters an unknown field id or field name while decoding a struct/union, it can continue by skipping that value and resuming with the rest of the stream. To allow this, fields and containers (map, set, list) contain the type code or token of the unknown type.
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
