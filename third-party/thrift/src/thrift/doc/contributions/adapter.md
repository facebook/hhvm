# Thrift Adapter

Thrift Adapter supports two conversions.
- Conversion from [standard type](../glossary/#kinds-of-types) to [adapted type](../glossary/#kinds-of-types)
- Conversion from adapted type to standard type
A conforming implementation **should** name these conversion `fromThrift` (or `fromThriftField` for Field Adapter) and `toThrift` for consistency.

Thrift Adapter **must not** affect wire format and **must** be isolated for each language.

## Type Adapter
[Type Adapter](../features/adapters#type-adapter) **must** support adapting typedef, field, or struct. Type Adapter **must** adapt once for each type. Type Adapter **must** support adapting an element or key in a container with typedef. Type Adapter **must** support conversion from adapted type to standard type and vice versa. Type Adapter **must** support conversion from adapted type to another adapted type when it is used for composition.

### Type Wrapper
An implementation **may** distinguish [Type Wrapper](../features/adapters#type-wrapper) from Type Adapter. Type Wrapper **must** not be used with Type Adapter on same typedef, field, or struct. Type Wrapper **must** support conversion from adapted type to standard type, but it **may not** support conversion from standard type to adapted type. Type Wrapper **must** support conversion from adapted type to another adapted type when it is used for composition.

### Type Adapter on Struct vs Typedef
An implementation **must** support adapting Type Adapter on struct. When a struct with Type Adapter is addressed in Thrift IDL, it addresses the adapted struct, and the raw struct without Type Adapter **must** not be addressible in Thrift IDL. Meanwhile, when a struct with typedef is adapted with Type Adapter, both raw struct and adapted struct **must** be addressible in Thrift IDL. Please refer to the user guide for the example.

## Field Adapter
[Field Adapter](../features/adapters#field-adapter) **must** support adapting field. Field Adapter **must** not support adapting typedef or struct. Field Adapter **must** accept field context including parent struct and field id. [`::apache::thrift::FieldContext`](https://github.com/facebook/fbthrift/blob/13da89c79097c864432ccf9ca1533318602b258e/thrift/lib/cpp/Field.h#L37-L41) is the field context C++ Field Adapter accepts during the conversion. Field Adapter **must** support conversion from adapted type to standard type and vice versa. Field Adapter **must** support conversion from adapted type to another adapted type when it is used for composition.

### Field Wrapper
An implementation **may** distinguish [Field Wrapper](../features/adapters#field-wrapper) from Field Adapter. Field Wrapper **must** not be used with Field Adapter on same field. Field Wrapper **must** support conversion from adapted type to standard type, but it **may not** support conversion from standard type to adapted type. Field Wrapper **must** support conversion from adapted type to another adapted type when it is used for composition.

## Validation
An implementation **should** use [Thrift Compiler validator](https://github.com/facebook/fbthrift/blob/13da89c79097c864432ccf9ca1533318602b258e/thrift/compiler/sema/standard_validator.cc#L154) to validate Thrift Adapter usage. It **must**  use the [scope validation annotation](https://github.com/facebook/fbthrift/blob/main/thrift/annotation/scope.thrift) to restrict the scope of Type Adapter and Field Adapter.

## Composition
An implementation **must** support composing Thrift Adapter. Thrift Adapter **must** use field to compose. Type Adapter on typedef or struct can compose with Type Adapter on field. Type Adapter on typedef or struct can compose with Field Adapter on field. Thrift Adapter **must** limit all other composition of Thrift Adapter. For example, a struct with Type Adapter cannot use typedef to compose another Type Adapter.

## Optimization
An implementation **may** provide customizations for [value operators](../features/operators.md), such as comparison, equality, hash, clear, serialized size, serialize, and deserialize. These customizations are optional, as Thrift Adapter **must** be able to always convert back to standard type. An implementation **should** follow the customizations described [here](../features/adapters#customization-for-optimization).

An implementation **may** provide in-place deserialization optimization for wrapper-like adapted types. During deserialization, Thrift Adapter requires to create a temporary instance of standard type, deserialize binary into the temporary instance, and call `fromThrift` to convert the temporary instance to the desired adapted type. Instead, the optimization can be performed by directly deserializing into internal standard value inside wrapper-like adapted types with `toThrift`. This requires Thrift Adapter to expose access to internal standard thrift value from `toThrift`.

## Certain details to note while implementing the APIs
* Each language **may** provide additional features in Thrift Adapter to support additional needs in each language.
* Type and Field Wrapper **may** not exist in some languages (C++ and Python), as these implementation does not differentiate Adapter and Wrapper.
* Thrift Adapter serves as a building block for many Thrift features, such as [Thrift Patch](../features/patch), Protocol Object, and more.
