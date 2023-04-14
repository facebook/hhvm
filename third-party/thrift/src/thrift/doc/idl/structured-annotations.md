# Structured Annotations

<!-- https://www.internalfb.com/intern/wiki/Thrift/Thrift_Guide/IDL/Annotations_(structured)/?noredirect -->

## Definition

[Read here](https://www.internalfb.com/intern/staticdocs/thrift/docs/idl/annotations)

## Exposure in languages

### Hack

The annotations are exposed on structs, exceptions, unions, services and constants via `getAllStructuredAnnotations` methods. Their return values are structured depending on the entity type. The leaf nodes of this data are annotations' key-value pairs. The key is the annotation struct's Hack name (with a namespace if applicable) and the value is the Thrift object with the annotation value.

Structs, exceptions and unions include annotations for themselves and for both fields and types (annotations from typedefs are propagated there). The getter method is defined in the corresponding structs, exceptions and unions.

Services include annotations for themselves and functions. The getter method is defined in a new `<SERVICE>StaticMetadata` class, which implements `\IThriftServiceStaticMetadata`. Services have many classes generated for them, so I didn't want to duplicate the annotations on every one.

Constants include annotations just for themselves. The getter method is defined on the existing `<MODULE>_CONSTANTS` class, which now implements `\IThriftConstants`.

### C++

Structured annotations are accessible in C++ via the [Thrift/Metadata](https://www.internalfb.com/intern/wiki/Thrift/Metadata/) API

### Python

Structured annotations are accessible in python via the [metadata api](https://www.internalfb.com/intern/wiki/Thrift_in_Python/User_Guide/Advanced_Usage/Metadata/). They are accessible on all objects which have metadata, which is structs, exceptions, unions, fields, services, and enums.

You can view an example for how to use these [here](https://www.internalfb.com/intern/diffusion/FBS/browse/master/fbcode/thrift/lib/py3/test/metadata.py?commit=80443af2713dbfa63ccd487d6d5f7d0850b2f022&lines=192).

### Java, Haskell, Erlang

Planned.

## Co-existence with unstructured annotations

Both types of annotations are supported at the moment. However, the unstructured annotations would be deprecated. Keep using the unstructured annotations to control the complier behavior for now, but switch to structured annotations for other use cases.
