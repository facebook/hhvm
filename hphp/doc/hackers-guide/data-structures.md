# HHVM Runtime Data Structures

This page contains a brief overview of the most important runtime data
structures in HHVM. It is not meant to be an exhaustive reference for all
details about these data structures, since those details change too often to
reliably document so far away from the code. Rather, reading the descriptions
here should leave you with a good high-level understanding of what each one is
used for and where to find the code implementing it for further investigation.

## Hack-visible values

### `DataType`, `Value`, and `TypedValue`

`DataType`, defined in [datatype.h](../../runtime/base/datatype.h), is an enum
representing the type of a Hack value: `KindOfNull` means `null`,
`KindOfBoolean` means `bool`, `KindOfInt64` means `int`, etc. Some user-visible
types are split into multiple `DataType`s to keep track of whether or not a
value is reference counted: `KindOfPersistentFoo` means "`KindOfFoo` that we
know is not reference counted." The reverse, however, is not true:
`KindOfPersistentFoo` is a subtype of `KindOfFoo`, so it is valid to use
`KindOfFoo` with a non-reference-counted Foo.

`TypedValue`, defined in [typed-value.h](../../runtime/base/typed-value.h)
represents a single Hack value, and appears in many different places in HHVM. It
contains a `DataType` and a `Value`, which is a union with one member for every
possible type. Primitive types (`null`, `bool`, `int`, and `float`) are stored
inline in the `TypedValue`, while all other types heap-allocate a data object
and store a pointer to it in the `TypedValue.`

### `ArrayData`

[`ArrayData`](../../runtime/base/array-data.h) is used to represent all
array-like types in Hack: `array`, `dict`, `vec`, and `keyset`, though you'll
never see any raw `ArrayData` objects created anywhere. Instead, a specific kind
of array is created and tagged using one of the [eight current array
`HeaderKind`
values](https://github.com/facebook/hhvm/blob/HHVM-3.27/hphp/runtime/base/header-kind.h#L46-L49).
We use a custom vtable to dispatch to the appropriate implementation for all
`ArrayData` member functions; the current implementation classes are
[`EmptyArray`](../../runtime/base/empty-array.h),
[`PackedArray`](../../runtime/base/packed-array.h),
[`MixedArray`](../../runtime/base/mixed-array.h),
[`SetArray`](../../runtime/base/set-array.h),
[`GlobalsArray`](../../runtime/vm/globals-array.h), and
[`APCArray`](../../runtime/base/apc-array.h). Note that many of these types
don't directly inherit from `ArrayData`, so they're only subtypes of `ArrayData`
by convention.

Arrays in Hack have value semantics, implemented using copy-on-write. As a
result, most member functions that perform mutations take a `bool copy`
parameter to indicate whether or not the array should be copied before
performing the mutation. It is up to the caller to call `cowCheck()` before any
mutations to determine if a copy is necessary. Additionally, any mutation could
cause reallocation of a new `ArrayData`, either to grow or escalate to a
different array kind. To support this, all mutation functions return a new
`ArrayData*`; the `ArrayData*` that was mutated should be considered dead and
you should use the new one in its place.

### `StringData`

[`StringData`](../../runtime/base/string-data.h) represents a Hack `string`
value. Like arrays, strings have value semantics using copy-on-write, so callers
are responsible for calling `cowCheck()` before mutating, although the copy must
be done manually with `StringData::Make()` rather than a `bool copy` parameter
to the mutator function.

Most `StringData`s store their data in space allocated immediately after the
`StringData`. This layout is required in `USE_LOWPTR`, so `StringData::m_data`
is [conditionally
defined](https://github.com/facebook/hhvm/blob/e05d2041a598ff655f594c4fec7e5f1708d9466b/hphp/runtime/base/string-data.h#L539-L542).
For normal builds, `m_data` will usually point right after the `StringData`, but
it may point elsewhere for strings from APC.

### `ObjectData`

[`ObjectData`](../../runtime/base/object-data.h) represents a Hack `object`. It
contains a `Class*` describing its type (described below) and a series of
attributes. Declared properties are stored in an array of `TypedValue`s that is
allocated after the `ObjectData`.

### `Smart pointer wrappers`

All of the types described so far have smart pointer wrappers that are generally
used for high-level non-performance-critical C++ code that needs to work with
one of them. The wrapper type can be found by dropping the `Data` suffix, so
`StringData`'s wrapper is [`String`](../../runtime/base/type-string.h). Note
that like most smart pointer types, these wrappers can all represent a `null`
value, but the pointer in a `TypedValue` representing an array, string, etc.
must *never* be `nullptr`. A `null` value is represented using `KindOfNull`.

## Runtime-internal data structures

### `Unit`

A [`Unit`](../../runtime/vm/unit.h) represents all of the information contained
in one Hack file: classes, functions, constants, top-level code, etc. All
references to entities that could be defined in another file are only referenced
by name, even if they are defined in the current file. This includes, but it not
limited to, function names in call expressions, parent class names, and used
traits. This is to support one of HHBC's core principles: it is always possible
to emit bytecode (and construct a `Unit`) for a single file in complete
isolation. If that file references entities that are undefined at runtime, the
appropriate error will be raised.

### `PreClass` and `Class`

Every `Unit` has a list of classes that are defined in the file it came from.
Each of these classes is stored as a [`PreClass`](../../runtime/vm/preclass.h),
which represents the class definition as it is visible in the source code.
Parent classes, used traits, and any other references to entities other than the
class itself are stored as string names.

When a class is defined at runtime, these references are resolved to concrete
entities, producing a [`Class`](../../runtime/vm/class.h). The resolved parent
class is stored as a `Class*` rather than the string name from the `PreClass`.

This two-level representation of classes is necessary because the mapping
between name and class can change with every request. All classes are redefined
from scratch in each request<sup>[1](#f1)</sup>, and different definitions of a
class can be selected by including different files, or by putting the
definitions on different control flow paths. Properly-typed Hack code will
never have multiple definitions of the same class, but HHVM still supports it.

### `Func`

All Hack functions, including methods on classes, are represented using a
[`Func`](../../runtime/vm/func.h). Each `Func` contains a pointer to the `Unit`
it was defined in, the `Class` it is a member of (if any), its location in the
`Unit`'s bytecode, information about its parameters, and various other metadata.

Every `Func` has a 32-bit `FuncId`, which is assigned in ascending order as each
`Func` is created. There is a global table that allows mapping from this ID back
to a `Func*`, and `FuncId`s are often used in places where we want to store a
reference to a `Func` without a full 64-bit pointer<sup>[2](#f2)</sup>.
`FuncId`s are also guaranteed to be unique for the lifetime of the process,
unlike `Func*`.

--------

<b id="f1">1:</b> This is what HHVM presents to the user, but we have
mechanisms to avoid the work of actually redefining everything in every request
when possible.

<b id="f2">2:</b> HHVM also has a `USE_LOWPTR` build mode that allocates certain
data structures, including `Func`s, in the lower 4GiB of address space, allowing
us to store 32-bit pointers using `HPHP::LowPtr<T>`. However, `LowPtr<T>` is 64
bits in non-`USE_LOWPTR` builds, and some uses of `FuncId` rely on it being 32
bits in all build modes.
