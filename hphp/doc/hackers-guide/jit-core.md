# HHVM JIT

HHVM's just-in-time compiler module is responsible for translating sequences of
[HHBC](../bytecode.specification) into equivalent sequences of x86-64 or ARM64
machine code. The vast majority of the code implementing the JIT lives in the
namespace `HPHP::jit`, in [hphp/runtime/vm/jit](../../runtime/vm/jit). Most
file and class names referenced in this document will be relative to that
namespace and path.

The basic assumption guiding HHVM's JIT is that while PHP is a dynamically
typed language, the types flowing through PHP programs aren't very dynamic in
practice. We observe the types present at runtime and generate machine code
specialized to operate on these types, inserting runtime typechecks where
appropriate to verify assumptions made at compile time.

## Region Selection

The first step in translating HHBC into machine code is deciding exactly which
code to translate, in a process called region selection. The size and shape of
regions depend on many factors, ranging from a single HHBC instruction up to an
entire function with complex control flow. There are a number of different ways
to select a region, all producing a `RegionDesc` struct in the end (defined in
[region-selection.h](../../runtime/vm/jit/region-selection.h)). `RegionDesc`
contains a list of `RegionDesc::Block` structs, and each `Block` represents a
basic block of bytecodes by holding a starting offset and length in
instructions. The list of `Block`s is kept sorted in reverse post order. Blocks
also contain optional metadata about the code they contain and the state of the
VM before, during, and after execution of that code. This metadata includes
type predictions, parameter reffiness predictions, statically known call
destinations, and certain postconditions.

### Tracelet Region Selection

The first-gear region selector is the tracelet region selector. The name
"tracelet" is somewhat vestigal, and refers to a region of HHBC typically no
larger than a single basic block. This code lives in
[region-tracelet.cpp](../../runtime/vm/jit/region-tracelet.cpp), and its goal
is to select a region given only the current live VM state - most importantly
the current program counter and the types of all locals and eval stack
slots. `HhbcTranslator` (see the "HHIR" section) is used to simulate execution
of the bytecode using types instead of values, continuing until a control flow
instruction is encountered or an instruction attempts to consume a value of an
unknown type. These rules are not absolute; there are a number of
exceptions. If an unconditional forward Jmp is encountered, the tracelet will
continue at the Jmp's destination. In addition, certain bytecodes (such as PopC
and IsTypeC) are able to operate on generic values and don't cause a tracelet
to terminate due to imprecise types.

### PGO and Hottrace Region Selection

HHVM's JIT supports

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
  Bool           | `false=0`, `true=1` (8 bits at runtime)
  Int            | `int64_t` (64-bit twos compliment binary integer)
  Dbl            | `double` (IEEE 754 64-bit binary floating point)
  StaticStr      | `StringData*` where `isStatic() == true`
  CountedStr     | `StringData*` where `isStatic() == false`
  Str            | `StringData*` `{CountedStr+StaticStr}`
  StaticArr      | `ArrayData*` where `isStatic() == true`
  CountedArr     | `ArrayData*` where `isStatic() == false`
  Arr            | `ArrayData*` `{CountedArr+StaticArr}`
  UncountedInit  | `TypedValue`: `{Null+Bool+Int+Dbl+StaticStr+StaticArr}`
  Uncounted      | `TypedValue`: `{Uninit+Null+Bool+Int+Dbl+StaticStr+StaticArr}`
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
  StkElem       | `{Gen+Cls}`

### Values, Instructions, and Blocks

An HHIR program is made up of `Blocks`, each containing one or more
`IRInstructions`, each of which produces and consumes zero or more `SSATmp`
values. These structures are defined in
[block.h](../../runtime/vm/jit/block.h),
[ir-instruction.h](../../runtime/vm/jit/ir-instruction.h), and
[ssa-tmp.h](../../runtime/vm/jit/ssa-tmp.h), respectively.

An `SSATmp` has a type, an SSA id, and a pointer to the `IRInstruction` that
defined it. Every `SSATmp` is defined by exactly one `IRInstruction`, though
one `IRInstruction` may define more than one `SSATmp`. Every instruction has an
`Opcode`, indicating the operation it represents, and a `BCMarker`, which
contains information about the HHBC instruction it is part of. Depending on the
opcode of the instruction, it may also have one or more `SSATmp*` sources, one
or more `SSATmp*` dests, one or more `Edge*`s to other `Blocks`, a `Type`
parameter (known as a typeParam), and an `IRExtraData` struct to hold
compile-time constants. `IRInstructions` are typically created with the
`IRBuilder::gen()` method, which takes an `Opcode`, `BCMarker`, and then a
variadic number of arguments representing the other properties of the
instruction.

A `Block` represents a basic block in a control flow graph. A pointer to one
`Block` is stored in `IRUnit` as the entry point to the program; all other
`Block`s must be reached by traversing the CFG. Certain instructions are "block
end" instructions, meaning they must be the last instruction in their `Block`
and they contain one or more `Edge`s to other `Block`s. `Jmp` is the simplest
block end instruction; it represents an unconditional jump to a destination
block. `CheckType` is an example of an instruction with two `Edge`s: "taken"
and "next". It compares the runtime type of its source value to its typeParam,
jumping to taken block if the type doesn't match, and jumping the next block if
it does.

While block end instructions may only exist at the end of a `Block`, there are
two instructions that may only exist at the beginning of a `Block`: `DefLabel`
and `BeginCatch`. `DefLabel` serves as a phi-node, joining values at control
flow merge points. `BeginCatch` marks the beginning of a "catch block", which
will be covered in more detail later.

### Control Flow

HHIR can represent arbitrary control flow graphs, though most optimization
passes do not currently support loops. In place of traditional phi-nodes, HHIR
uses `Jmp` instructions that take sources and pass them to `DefLabel`
instructions. Consider the following program that performs some numerical
computation:

```
B1:
  t1:Int = ...
  JmpZero t1:Int -> B3
 -> B2

B2:
  t2:Int = AddInt t1:Int 5
  Jmp t2:Int -> B4

B3:
  t3:Dbl = ConvIntToDbl t1:Int
  t4:Dbl = MulDbl t3:Dbl 3.14
  Jmp t4:Dbl -> B4

B4:
  t5:{Int|Dbl} = DefLabel
  ...
```

After control flow splits at the end of B1, B2 and B3 each do their own
computation and then pass the result to the `DefLabel` at the join point,
B4. This is equivalent to the following phi-node: `t5:{Int|Dbl} = phi(B2 ->
t2:Int, B3 -> t4:Dbl)`

## Optimizations

A number of basic optimizations are performed on each instruction by
`IRBuilder` as it is generated and added to its `Block`: common sub-expression
elimination, pre-optimization, and simplification. The CSE is fairly standard:
the instruction and its sources are checked against a hash table to determine
if the entire instruction can be replaced by an existing instruction performing
the same computation. Pre-optimization performs simple optimizations based on
tracked state in `IRBuilder`, such as replacing a `LdLoc` instruction with a
use of an already known value for that local variable. Simplification comes
last, and contains any optimizations that only require inspecting an
instruction and its sources. This is primarily constant folding but also
includes things like eliminating `CheckType` instructions that are known at
compile-time to succeed. Both pre-optimization and simplification may return an
already-defined value to use instead of the new instruction, or they may
generate one or more different instructions to use in place of the original.

Once the initial translation pass is complete, a series of more involved
optimizations are run on the entire CFG. These are described in detail in
[jit-optimizations.md](jit-optimizations.md).

## Machine Code Generation

### Register Allocation

### Code Generation

#### x86-64

#### ARM64
