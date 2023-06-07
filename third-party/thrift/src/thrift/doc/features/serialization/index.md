# Serialization

<!-- https://www.internalfb.com/intern/wiki/Thrift/Overview/Serialization/?noredirect -->

## Protocols

There are two approaches that may be used for serialization:

* Serialization by field id
   * In this case, the serialized data contains the id, the type, and the value for every field that is serialized. The name is not included in the serialized data. If the field is of an enumeration type, the integer value of the enumerator is included in the serialized data, and the enumerator (the named constant) is not included.
   * Compact/Binary/Frozen protocols use this approach, which are used by most thrift services at Meta.
* Serialization by field name
   * In this case, the serialized data contains the name, the type, and the value for every field that is serialized. The id is not included in the serialized data. If the field is of an enumeration type, the enumerator (the named constant) is included in the serialized data, and the integer value of the enumerator is not included.
   * JSON protocol uses this approach. Most notable use cases at Meta are Configerator and Tupperware to generate configs in a human readable JSON format.

*Caution: Never mix serialization by field id and serialization by field name within the same use case.*

## Qualifiers

For different qualifiers, the serialization behavior is slightly different:

* Optional field is only serialized when this field is set. (Note: in C++, due to historical reason, by using deprecated API (e.g. `value_unchecked()`), it is possible to change underlying value without setting the field).
* Unqualified field is always serialized. If data is not set, it will be serialized to default value. This is recommended unless you need to distinguish empty and the default value.
* Required field is always serialized since it’s always set. You can’t unset it. **This qualifier is deprecated and most languages ignore it**, don't use it in new code.

```
struct ThriftStruct {
  1: string unqual_field; // unqualified
  2: optional string opt_field; // optional
  3: required string req_field; // required
}
```

The representation of required, optional, and unqualified fields in serialized data are identical.

## Class type

The serialized data for structs, unions, and exceptions are indistinguishable.

---

# Deserialization

## Protocols

When serialized data is deserialized into a struct object, the fields in the serialized data are *matched* to fields in the struct object:

* If serialized by id, fields with the same id are matched.
* If serialized by name, fields with the same name are matched.

The value of the field in the serialized data is assigned to the matching field in the struct object, thus

1. After deserialization, this field in the struct object will be *present* with that value.
2. Any unmatched fields in the serialized data are ignored.
3. Any matched fields with mismatched types in the serialized data are ignored.
4. For given field in struct object, if there is no matching fields in the serialized data, it remains untouched.

## Qualifiers

The deserialization behavior is same for all qualifier. However, the initialization behavior is slightly different:

* Optional field will be empty. Attempting to access this field results exception.
* Unqualified field will be empty. Accessing it results default value.
* Required field will be set to default value.

For RPC, since we are not only deserializing the payload, but also initializing thrift object, different qualifier changes RPC behavior when sending/receiving thrift request.

## Class type

Deserialization into an union object fails when there is more than one match between the serialized data and the union object. Otherwise structs, unions, and exceptions have same behavior.
