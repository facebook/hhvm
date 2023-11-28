# Terse Write

## Background

`@thrift.TerseWrite` is a new structured [annotation](../idl/annotations#thrift-annotations) that will mark a field as `terse`.  Please refer to the [user guide](../features/terse-write) for the motivation.

## Semantic

Please refer to the [spec](../idl/field-qualifiers#fields-annotated-with-thrifttersewrite) for more information.

### Serialization

If a terse field equals to the [intrinsic default value](../idl/#default-values), it will be skipped during the serialization.  A terse struct field is skipped serialization, if all of the nested fields are empty.


:::note

`unqualified` and `required` fields can never be empty as they are always serialized.

:::
:::note

`optional` field is eligible for skipping serialization if it wasn’t explicitly set

:::

For non-structured types, we perform a check before serialization of each field whether it is equal to the intrinsic default value or not. For an optimization, we recommend to avoid this for structured types. Instead, we perform an empty check after serializing all nested fields and check whether any field was written into a buffer. If none of nested fields were written into a buffer, we recommend to rollback metadata (i.e.  field id, T_STRUCT, and T_STOP) that is already written in the buffer.

### Deserialization

If a terse field is not part of the serialized binary, there could be two possible reasons.

1. The field equals to the intrinsic default value, and the serialization was skipped.
2. The field was newly added, and it was not included in the binary that serialized the struct.

Therefore, a terse field always need to be set to intrinsic default before deserialization even for a terse field with the custom default. For a nested structured type, we only clear nested terse fields before deserialization to keep the semantic other qualifiers. Please refer to the [user guide](../features/terse-write/#custom-default) for examples.



### Union

Union is a special structured type where a single field can be set at any time. Therefore, fields inside union can be treated as if they have optional qualifiers. Therefore, `@thrift.TerseWrite` cannot annotate fields inside union.

### Exception

Thrift Exception is treated identical to Thrift Struct. Users can use `@thrift.TerseWrite` on fields on Thrift Exception to mark the fields as terse.

## Certain details to note while implementing the APIs

* `@thrift.TerseWrite` can be applied on the package-level, structured-level, and field-level. For package-level, all unqualified fields in Struct or Exception in the thrift file are promoted to terse fields. For structured-level, all unqualified fields within the specific Struct or Exception are promoted to terse fields. For field-level, the specific unqualified field is promoted to terse field. Please refer to the [user guide](../features/terse-write) for examples.

* `@thrift.TerseWrite` changes the field qualifier in `t_field` directly. Therefore, `t_field_qualifier::terse` can be used in Thrift AST to determine whether a field is terse or not. Each language implementor does not need to check the existence of the annotation.
* There is no need to touch Thrift Compiler; please make an appropriate changes in codegen for consuming Thrift terse fields.
* A terse field does not distinguish whether it is explicitly set or not. Therefore, it does not require to have an extra bit of information to track whether a field was explicitly set or not. For example, in C++, a user can check whether an unqualified field is explicitly set by user using `obj.unqualified_field().is_set()`, we do not expose this API to a terse field.
