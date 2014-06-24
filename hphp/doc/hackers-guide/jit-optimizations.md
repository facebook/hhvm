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
is managed by the [IRBuilder](../../runtime/vm/jit/trace-builder.h) class. The
important methods in this process are `IRBuilder::constrainValue()`,
`TraceBuidler::constrainLocal()`, and `IRBuilder::constrainStack()`. Whenever
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

## Refcount Optimizations

### Goals

PHP has automatic memory management using reference counting, and this is baked
into the semantics of HHVM. In addition to the references visible to PHP
developers, such as those in local variables and object properties, a
significant number of additional references to objects are created and
destroyed as they're pushed to and popped from the evaluation stack. Many of
these operations can be elided without visibly affecting program behavior, and
doing so is the responsibility of the refcount optimizations, implemented in
[refcount-opts.cpp](../../runtime/vm/jit/refcount-opts.cpp).

Consider the following sequence of bytecodes, which could result from the PHP
expression `$a[4] = $b`:

```
CGetL 1
SetM <L:0 EI:4>
PopC
```

The CGetL loads `$b`'s value and pushes it on the VM stack, creating an
additional reference to it. The SetM stores the value from the top of the stack
to index 4 of the array in `$a`, creating one more reference to it, and pushes
the value it stored back on the VM stack. Nothing is using the result of the
assignment expression, so the PopC bytecode discards the value, destroying a
reference to it. After all three bytecodes have executed, the refcount of
`$b`'s value has increased by one, representing the new reference inside
`$a`. However, we performed three refcounting operations to achieve this
effect, when one will suffice. This optimization pass is able to prove that we
can safely eliminate one incref operation and one decref operation without
visibly affecting program behavior.

### Refcount producers, consumers, and validation

Memory management is a tricky subject, and getting it wrong can lead to subtle
use-after-free bugs or memory leaks. Additionally, PHP offers user-defined
object destructors that are run exactly when an object's refcount hits 0, so
leaking an object can visibly affect program behavior. To help prevent these
issues, the refcount optimization pass also includes a refcount operation
validator. There are two ways to determine the number of references to an
object: its `m_count` field and the number of physical pointers to the object,
which we will call NPTRS. These two values are usually exactly the same,
reflecting the fact that m_count keeps track of the number of physical
references to the object. If, upon leaving a translation, an object's m_count
field is greater than NPTRS, we have created a memory leak. If NPTRS is greater
than m_count, we have created the potential for early destruction and a
use-after-free bug. We must therefore ensure that any differences between
m_count and NPTRS are carefully managed and resolved when appropriate. The
validation code uses the concepts of reference producers and consumers to
ensure that neither of these problems exists, either as a result of the
optimization itself or the structure of the IR given to the optimization.

A refcount producer is an operation that either increases an object's m_count
field by one or decreases NPTRS by one. Note that decreasing NPTRS by one
doesn't always involve physically writing over the memory containing the
pointer; it might just become invalid because its containing object was freed
or the frame it was contained in was unwound. Example refcount producers are
the `IncRef` instruction, which increases m_count by one, and the
`LdUnwinderValue` instruction, which loads an object from thread local storage,
taking ownership of the pointer.

A refcount consumer performs the reverse: it either decrements m_count or
increments NPTRS by creating a new pointer to the object. Example refcount
consumers are `DecRef`, which decreases m_count by one, and `StLoc`, which
stores a pointer to an object in a local variable.

The refcount validator checks that in all control flow paths, there are exactly
zero unconsumed references to all objects. This means that each instruction
producing a reference must be paired with an instruction that consumes that
reference. Common pairings are `IncRef; StProp` or `CGetElem; StLoc`.

### Refcount observers

While refcount producers and consumers are useful for validation refcount
operations, the important concept for optimizing these operations is the
refcount observer. This is any HHIR instruction that inspects an objects's
reference count and takes some action based on what it sees. One of the most
common observers is `DecRef`. `DecRef` first inspects an object's reference
count. If it's equal to 1, it destroys the object; otherwise, it decrements the
refcount. Therefore, this optimization must not make any modifications that
would cause a `DecRef` instruction to see a different answer to the question
"is this object's refcount equal to 1?"

### Implementation

Armed with knowledge about which HHIR instructions are refcount producers,
consumers, and observers, the bulk of this optimization is an analysis pass on
the IR that performs the previously mentioned validation while looking at
`IncRef` instructions and determining how far down in the instruction stream
each one can safely be moved. It also tracks and records lower bounds on the
refcount of each object that is a source to a `DecRef` instruction. This
analysis happens in the `SinkPointAnalyzer` class, beginning in
`SinkPointAnalyzer::find()`. A "sink point" is just a position in the
instruction stream that is a potential destination of an `IncRef` instruction.

The blocks of the CFG are traversed in reverse postorder and each instruction's
effects on the number of unconsumed references to each live value is
tracked. If you start reading the code in `SinkPointAnalyzer` you will notice
that two simplifying assumptions have been made:

1. All refcount consumers are also treated as observers. This is conservatively
   correct, so it shouldn't cause any incorrect operations but we may miss out
   on some optimizations. We make this simplification because all observers are
   consumers and many consumers are also observers. Instructions that are
   consumers but not observers are treated suboptimally, but the simplification
   this allows in the analysis code was worth the tradeoff when this
   optimization was first written. This may change in the future.
2. The number of unconsumed references to an object is used as the lower bound
   on its refcount, instead of tracking both independently. Similar to #1, this
   is conservatively correct and may change in the future. For now, though, it
   greatly simplifies the state tracking necessary as well as the effect flags
   we need to maintain on each HHIR instruction.

The analysis is followed by a simple mutation pass, implemented in
`sinkIncRefs` and `eliminateRefcounts`. First, any `DecRef` instructions taking
a source with a refcount of at least 2 are converted to `DecRefNZ`, since we
know the object's refcount will be non-zero after the operation. Second, any
`IncRef`s that can safely be moved to be adjacent to a `DecRefNZ` of the same
object are moved, then the pair is erased. We know this is safe because the
`DecRefNZ` operation can't cause destruction of the object, meaning the
sequence `IncRef t0; DecRefNZ t0` is always a no-op. The exact details of how
this mutation happens are slightly different for efficiency reasons, but the
end result is the same.

## Jump Optimizations

## Dead code elimination

## Reoptimize Pass
