# Universal Name

This guide provides details how universal name can be supported in a language. Universal name is mainly used in conformance Any, thrift Any and SemiAny. Refer to the [user guide](../features/universal-name) for details. A major component for universal name is the Type Registry.

## Type registry
Type registry provides different mappings between hash to type, universal name URI to type and type to URI/hash.

An application **should** able to resolve a type from a given hash. This could be a 16 byte hash or just the prefix of the hash value. A sorted map data structure or any variant of sorted map (i.e ConcurrentSkipListMap in Java) is ideal to store the hash-type mapping and can access the type with O(log(n)) time complexity. There can be many thrift objects (types) defined in large deployments, using a cache for hash lookup will eliminate wasting CPU cycles and provides O(1) access. If the hash prefix result multiple type mappings, this may happen when the hash value is too short or hash conflict occurs, Type Registry throws an exception.

Type to hash mapping stores 16 byte of hash prefix of the type and prevents hash re-calculation.

Java [TypeRegistry](https://github.com/facebook/fbthrift/blob/main/thrift/lib/java/common/src/main/java/com/facebook/thrift/type/TypeRegistry.java) implementation can be used as a reference.

## Registering universal names
All the universal names, mapping to the thrift types, **must** be registered to the `Type Registry` to resolve a type from given hash prefix. Iterating through all thrift objects might not be practical when an application start. A better approach is to read all thrift objects during codegen, generate code to register these objects automatically when the application starts.

## Validation
Universal names are validated by the thrift compiler during compile time. Any URI that does not match the rules defined in the [spec](../features/universal-name#universal-names) cause an error. Supported thrift languages **should not** do any extra validation and assume a valid URI is passed to the application.
