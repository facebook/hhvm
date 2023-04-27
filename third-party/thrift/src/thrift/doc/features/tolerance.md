# Tolerance

This category of conformance describes the expected behavior when a reader reads malformed data. It describes what type of behavior is expected when malformed data is encountered. It is important to verify that these cases are handled gracefully in all Thrift implementations as serialized data may be unintentionally or intentionally altered before deserialization.

## Behaviors

* Exception: ProtocolException will be thrown.
* Corrupted data: Reader returns data that might be different from what's serialized.
* Exception or corrupted data: Either ProtocolException will be thrown, or reader returns incorrect data.

:::note
This document covers aspirational behavior of existing Thrift implementations.
Thrift does not currently support all cases below
:::

## Cases

 | Issue                                        | Result                      |
 | -------------------------------------------- | --------------------------- |
 | Incorrect length for containers              | Exception or corrupted data |
 | Empty varint bytes                           | Exception or corrupted data |
 | Unknown type                                 | Exception                   |
 | Multiple equivalent set values               | Corrupted data              |
 | Multiple equal set values                    | Corrupted data              |
 | Multiple equivalent map keys                 | Corrupted data              |
 | Multiple equal map keys                      | Corrupted data              |
 | Out of range for integer                     | Exception                   |
 | Invalid UTF-8 in string field                | Exception                   |
 | Union with multiple fields set               | Exception                   |
