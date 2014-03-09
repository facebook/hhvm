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

Once a region has been selection, it is translated into the [HipHop
Intermediate Representation](../ir.specification), commonly referred to as
HHIR. HHIR is an SSA-form, strongly-typed intermediate representation
positioned between hhbc and machine code.

### Type System

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
