# HHVM JIT Optimization Passes

## Guard Relaxation

By default, every live program location read in a translation creates a type
guard. There are currently 10 different primitive types that can be guarded on,
so a translation with just 4 guards can have up to 10<sup>4</sup> unique
combinations of input types, leading to a combinatorial explosion in the number
of retranslations required to support all runtime type combinations. This is
bad news for both JIT code size and runtime performance - these retranslations
chain linearly to each other, so the time it takes to make it past the guards
is O(nTranslations) in the worst case. The runtime option
`Eval.JitMaxTranslations` limits the number of translations allowed per
`SrcKey`, and once this limit is hit any further retranslation requests will
result in a call out to the interpreter. This is almost always less desirable
than generating slightly suboptimal machine code in one or more of the
translations, allowing it to accept a variety of input types. The process we
use to determine which type guards can be loosened is called guard relaxation.

There are two parts to guard relaxation: value constraining and the relaxation
itself. Value constraining happens during the initial IR generation pass, and
is managed by the [IRBuilder](../../runtime/vm/jit/ir-builder.h) class. The
important methods in this process are `IRBuilder::constrainValue()`,
`IRBuilder::constrainLocal()`, and `IRBuilder::constrainStack()`. Whenever
the behavior of a sequence of HHIR depends on the types of one or more values,
the types of those values must be constrained using the constrain* methods
previously mentioned. Each takes the value to be constrained and how it should
be constrained. The possible constraints are defined in the `DataTypeCategory`
enum in [datatype.h](../../runtime/base/datatype.h) and are defined in order of
ascending specificity:

* `DataTypeGeneric` indicates that the type of the value does not matter and is
  equivalent to not constraining the value at all. This is most often used for
  values that are simply moved from place to place.
* `DataTypeCountness` indicates that the only use of the value is to incref or
  decref it. If the value's type is not refcounted, the type guard may be
  relaxed to `Uncounted`.
* `DataTypeCountnessInit` is similar to `DataTypeCountness`, with the exception
  that guards for the type `Uninit` will not be relaxed. This is most commonly
  used for bytecodes like `CGetL`, where `Uninit` values cause a notice to be
  raised while all other uncounted types are treated equally.
* `DataTypeSpecific` is the default constraint, and indicates that the guard
  should not be relaxed past a specific `DataType`.
* `DataTypeSpecialized` indicates that in addition to requiring the value's
  `DataType`, there is an additional type tag in the value that must be
  checked. Currently this includes object classes and array kinds, used by
  [MInstrTranslator](../../runtime/vm/jit/minstr-translator.cpp) to emit more
  efficient machine code for some container objects.

The guard relaxation process often needs to track additional information beyond
a single `DataTypeCategory`. The `TypeConstraint` struct, defined in
[type.h](../../runtime/vm/jit/type.h) is used to hold this information. A
value's type constraint is typically specified by passing a `TypeConstraint`
(`DataTypeCategory` implicitly converts to `TypeConstraint`) to value accessor
methods like `HhbcTranslator::popC()` and `HhbcTranslator::ldLoc()`. The former
is used extensively and so its `TypeConstraint` parameter is optional,
defaulting to `DataTypeSpecific`.

`TypeConstraint`, when used with `DataTypeSpecialized`, also requires
additional information about what property of the type is desired. This
information can be specified using the `setWantArrayKind()` and
`setDesiredClass()` methods.

Note that **any** decisions made in the JIT based on a value's type must be
reflected in that value's type constraint. This includes situations where the
absence of code depends on a value's type, such as eliding refcounting
operations on non-refcounted values. Typically, this just means using
`DataTypeSpecific` and giving no further thought to guard relaxation. If,
however, the operation you are translating can tolerate less specific types,
use an appropriate type constraint and ensure that any HHIR opcodes emitted can
tolerate having the types of their inputs loosened.

When `IRBuilder` is instructed to constrain the type of a value, it walks up
the chain of instructions leading to the value's definition, looking for the
instruction that determined the value's type. Sometimes this instruction is an
opcode with a known output type, such as `ConcatStrStr` which always produces a
`Str`. In these cases nothing is constrained, since the value's type does not
depend on a guard. When the source of a value's type is found to be a guard
instruction such as `GuardLoc`, `GuardStk`, or `CheckType`, the current
constraint for that guard is tightened according to the new constraint. All
guards start as `DataTypeGeneric` and may only have their constraints
tightened. The constraints for all guards are stored in a `GuardConstraints`
object owned by `IRBuilder`.

Certain optimizations performed by `Simplifier` are disabled during the initial
IR generation pass. The best example of this pattern is the `IncRef`
opcode. When given an input value of an uncounted type, the IncRef can be
eliminated. However, if the input type of the instruction may be loosened by
guard relaxation, it must not be eliminated. The reoptimize pass, described
below, eliminates instructions that are still eligible for simplification after
guard relaxation runs.

Once initial IR generation is complete, guard relaxation is the first
optimization pass to run. This is to simplify the other passes: any
modifications of the IR performed before guard relaxation would have to be
careful to keep the `GuardConstraints` map updated, and the loosened types
produced by guard relaxation may affect the behavior of other passes. The
relaxation pass is implemented in `relaxGuards()` in
[guard-relaxation.cpp](../../runtime/vm/jit/guard-relaxation.cpp). It is a
fairly simple pass: for all guard instructions present in the trace, their
constraint is looked up in the `GuardConstraints`. If the type of the guard is
more specific than is required by its constraint, the type is loosened. Some
guards will be loosened all the way to `Gen`; these guards will be eliminated
in the reoptimize pass. After loosening guard types as needed, one more pass
over the trace is performed to recompute any types derived from the modified
guards.

Most code in the JIT doesn't have to care about guard relaxation. If you're
implementing an operation that is type agnostic (or can be made type agnostic
without bloating the code much), it may be a good candidate for something more
general than `DataTypeSpecific`. Be aware that guard relaxation's effect on the
generated code is almost always negative, so it should only be used in
situations where the benefits of having fewer translations for the current
`SrcKey` outweigh the increased complexity of the generated code.

## Dead code elimination
