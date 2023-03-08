# Thrift Adapter
Thrift Adapter is the standard customization method to allow users to use their own custom types on Thrift generated structs and fields. Thrift Adapter **must** be able to convert from [standard type](../../glossary/#kinds-of-types) to [adapted type](../../glossary/#kinds-of-types) and vice versa. Thrift Adapter is designed so that it can be enabled per each langauge without intefering wire format. Thrift Adapter serves as a building blocks for other Thrift features, such as [Thrift Patch](patch.md) and Thrift Any.

Thrift Adapter **may** provide customization for [value properties](value#properties), including comparison and equality, and [value operators](value#operators), including hash, clear, serialized size, serialize, and deserialize. This allows Thrift Adapter to avoid conversion back to standard type for evaluation and manipulation.

## Type Adapter
Type Adapter **must** accept standard type and convert to adapted type. It **must** convert from standard type to adapted type and from adapted type to standard type. Type Adapter **must** convert from adapted type to another adapted type when it is used for composition.

### Type Wrapper
Type Wrapper is a special case of Type Adapter, where adapted type is restricted to be a wrapper around standard type. Type Wrapper **must** convert from adapted type to standard type. Type Wrapper **may** convert from standard type to adapted type. Type Wrapper **must** convert from adapted type to another adapted type when it is used for composition. Each language **may** distinguish Type Adapter and Type Wrapper.

## Field Adapter
Field Adapter **must** accept standard type and field context which consists of parent struct and field id. Field Adapter **must** convert from standard to adapted type and from adapted type to standard type. Field Adapter **must** convert from adapted type to another adapted type when it is used for composition.

### Field Wrapper
Field Wrapper is a special case of Field Adapter, where adapted type is restricted to be a wrapper around standard type. Field Wrapper **must** convert from adapted type to standard type. Field Wrapper **may** convert from standard type to adapted type. Field Wrapper **must** convert from adapted type to another adapted type when it is used for composition. Each language **may** distinguish Field Adapter and Field Wrapper.
