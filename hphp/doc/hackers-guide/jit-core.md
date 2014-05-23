# HHVM JIT

HHVM's just-in-time compiler module is responsible for translating sequences of
[HHBC](../bytecode.specification) into equivalent sequences of x86-64 or ARM64
machine code. The vast majority of the code implementing the JIT lives in the
namespace `HPHP::JIT`, in [hphp/runtime/vm/jit](../../runtime/vm/jit). Most
file and class names referenced in this document will be relative to that
namespace and path.

The basic assumption guiding HHVM's JIT is that while PHP is a dynamically
typed language, the types flowing through PHP programs aren't very dynamic in
practice. We observe the types present at runtime and generate machine code
specialized to operate on these types, insert runtime typechecks where
appropriate to verify assumptions made at compile time.

## Region Selection

The first step in translating some HHBC into machine code is deciding exactly
which code to translate, in a process called region selection. The size and
shape of regions depend on many factors, but they are typically no more than
one basic block. There are a number of different ways to select a region, all
producing a `RegionDesc` struct in the end (defined in
[region-selection.h](../../runtime/vm/jit/region-selection.h)). `RegionDesc`
contains a list of `RegionDesc::Block` structs, and each Block represents a
basic block of bytecodes by holding a starting offset and length in
instructions. The list of Blocks must be kept sorted in reverse post
order. Blocks also contain optional metadata about the code they contain and
the state of the VM before, during, and after execution of that code. This
metadata includes type predictions, parameter reffiness predictions, statically
known call destinations, and certain postconditions.

### Tracelet Region Selection

The primary region selection method is known as the tracelet region selector
(for the source of the name "tracelet," read the "Legacy Analysis Code"
section). This code lives in
[region-tracelet.cpp](../../runtime/vm/jit/region-tracelet.cpp), and its goal
is to select a simple region using nothing but the live VM state.

### Hottrace Region Selection

### Method Region Selection


## Region Translation

## HHIR

Once a bytecode region has been selected, it is translated into [HipHop
Intermediate Representation](../ir.specification), commonly referred to as
HHIR. HHIR is an SSA-form, strongly-typed intermediate representation
positioned between HHBC and machine code. A few different classes are involved
in this translation:

* `HhbcTranslator`: Defined in
  [hhbc-translator.h](../../runtime/vm/jit/hhbc-translator.h), this class
  contains roughly one `emitXXX` method for each supported HHBC instruction. It
  decides which HHIR instructions to emit based primarily on the types of the
  inputs to each bytecode.
* `IRBuilder`: Defined in [ir-builder.h](../../runtime/vm/jit/ir-builder.h),
  this class is responsible for tracking state during symbolic execution and
  some very basic optimizations.
* `IRUnit`: Defined in [ir-unit.h](../../runtime/vm/jit/ir-unit.h), this class
  is responsible for creating the runtime data structures that represent HHIR
  instructions, values, and blocks.
* `Simplifier`: Defined in [simplifier.h](../../runtime/vm/jit/simplifier.h),
  this class is responsible for performing very basic optimizations such as
  constant folding/propagation or anything else that only requires inspecting
  an instruction and its sources.

### Type System

All values in HHIR have a type, represented by the `Type` class in
[type.h](../../runtime/vm/jit/type.h). A `Type` may represent a primitive type
or any arbitrary union of primitive types. Primitive types exist for
PHP-visible types such as `Int`, `Obj`, and `Bool`, and runtime-internal types
such as `FramePtr`, `Func`, and `Cls`. Primitive types also exist for PHP
references and pointers to PHP values: for each primitive PHP type `T`,
`BoxedT`, `PtrToT`, and `PtrToBoxedT` also exist. A number of types commonly
thought of as primitives are actually unions: `Str` is the defined as
`{StaticStr+CountedStr}` and `Arr` is defined as `{StaticArr+CountedArr}`. The
`Type` class has static data members for easy construction of all primitive
types and many common union types such as `Uncounted`, `Cell`, and `Gen`.

In addition to arbitrary unions of primitive types, `Type` can also represent
constant values and "specialized" types. A constant `Type` may represent the
integer 5 or the string "Hello, world!", while a specialized type can represent
an object of a specific class or an array of a specific kind.

Since types represent sets of values, we define relations on types in terms of
the sets of values they represent. Two types `S` and `T` are equal (`S == T`)
if they represent equal sets of values. `S` is more refined than `T` (`S <= T`),
or a subtype of `T`, if the set of values represented by `S` is a subset
of the set of values represented by `T`. `S` and `T` are not related if their
intersection is the empty set (also called `Bottom`).

As previously mentioned, types in HHIR represent a mix of PHP-visible types and
internal types. The following table describes types representing PHP
values. Note that the types used here are more specific than what can be
discriminated by user code (e.g., StaticStr and CountedStr both appear as type
string at the PHP level).

  Type           | HHVM representation
  ---------------|-------------------
  Uninit         | `KindOfUninit`
  InitNull       | `KindOfNull`
  Null           | `{Uninit+InitNull}`
  Bool           | `false=0`, `true=1` (actual bit width varies)
  Int            | `int64_t` (64-bit twos compliment binary integer)
  Dbl            | `double` (IEEE 754 64-bit binary floating point)
  StaticStr      | `StringData*` where `isStatic() == true`
  CountedStr     | `StringData*` where `isStatic() == false`
  UncountedInit  | `TypedValue`: `{Null+Bool+Int+Dbl+StaticStr+StaticArr}`
  Uncounted      | `TypedValue`: `{Uninit+Null+Bool+Int+Dbl+StaticStr+StaticArr}`
  Str            | `StringData*` `{CountedStr+StaticStr}`
  StaticArr      | `ArrayData*` where `isStatic() == true`
  CountedArr     | `ArrayData*` where `isStatic() == false`
  Arr            | `ArrayData*` `{CountedArr+StaticArr}`
  Obj            | `ObjectData*`
  Obj<Class>     | `ObjectData*` of the specific type Class
  Counted        | `{CountedStr+CountedArr+Obj+BoxedCell}`
  Cell           | `{Null+Bool+Int+Dbl+Str+Arr+Obj}`

A PHP reference is implemented as a container object (`RefData`) which contains
one value. The contained value cannot be another PHP reference. For every type
T in the table above, there is a corresponding type BoxedT, which is a pointer
to a RefData struct containing a value of type T.

  Type           | HHVM representation
  ---------------|--------------------
  BoxedInitNull  | `RefData*` containing `InitNull`
  ...            | Everything from the first table above can be boxed

Finally, there is one top-level type representing all possible PHP values:

  Type           | HHVM representation
  ---------------|--------------------
  Gen            | `{Cell+BoxedCell}`

The VM also manipulates values of various internal types, which are never
visible at the PHP semantic level, and do not have any type relation with the
above PHP-facing types.

  Type           | HHVM representation
  ---------------|--------------------
  PtrToT         | Exists for all T in `Gen`. Represents a `TypedValue*`
  Bottom         | No value, `{}`. Subtype of every other type
  Top            | Supertype of every other type
  Cls            | `Class*`
  Func           | `Func*`
  VarEnv         | `VarEnv*`
  NamedEntity    | `NamedEntity*`
  Cctx           | A `Class*` with the lowest bit set (as stored in `ActRec::m_cls`)
  Ctx            | `{Obj+Cctx}`
  RetAddr        | Return address
  StkPtr         | Pointer into VM execution stack
  FramePtr       | Pointer to a frame on the VM execution stack
  TCA            | Machine code address
  Nullptr        | C++ `nullptr`

There is also one special type which represents all the possible types that can
be on the VM evaluation stack: all PHP-visible types plus the runtime-internal
`Cls` type.

  Type          | HHVM representation
  --------------|--------------------
  StackElem     | `{Gen+Cls}`

### CFGs


## Machine Code Generation

### Register Allocation

### Code Generation

#### x86-64

#### ARM64

## Legacy Analysis Code

The `Translator` class, in
[translator.cpp](../../runtime/vm/jit/translator.cpp) contains a mixture of
legacy code that's on the way out and new code intended to be its
replacement. `Translator::analyze()` performs roughly the same duties as the
tracelet region selector, using an older, simpler IR.
