# Lesson 2: Understanding Hack bytecode

## Lesson goals:

* Learn how we evaluate Hack expressions using a stack machine.
* See how we implement operators, function calls, and array and object constructors in bytecode.
* Consider how alternative bytecode sequences could result in performance improvements.
* Write Hack code that compiles to specific bytecodes by [reading our documentation](../bytecode.specification).

---

## Step 0: Pull, update, and re-compile HHVM

As in the [previous lessons](lesson0.md), let's start by re-compiling HHVM as
we review some concepts.

---

## Bytecode for a stack machine

Hack bytecode is a linear sequence of instructions that implements the logic of
a Hack program. The main challenge of compiling Hack source code to bytecode is
to flatten an abstract syntax tree (AST) into this form. Compiling for a stack
machine is one natural way to do this flattening.

Let's work through an example. Here's the Hack expression that we're going to
compile to bytecode:

```
$a * f($b, 2, 'x') + $c
```

We're going to assume that we have a working parser for this language, so we
can parse this expression into a tree format. Furthermore, we're going to
assume that this parser handles order of operations for us, so that the root
node of this tree is the "add" operation, even though that operation comes
second. (Note that the root node of the tree is the **last** expression
evaluated!) Here's an AST for this expression:

```
      Add
     /   \
   Mul    $c
  /   \
$a      Call: f
       /   |   \
     $b    2    $x
```

Our goal is to produce Hack bytecode that will push the result of an expression
on the "stack". Here, the stack is an abstract construct of HHVM's virtual
machine. It does not necessarily correspond to a computer's native stack, and
depending on how we compile a function to machine code, we may even optimize
away all access to this stack! However, we can define HHVM's behavior in terms
of this abstract stack.

Let's call the function that produces bytecode for an AST node
"CompileToBytecode". We can implement this function recursively. In pseudocode,
that might look like:

```
def CompileToBytecode(ast_node, bytecode):
  if not ast_node.children:
    bytecode.push(CompileLeaf(ast_node.leaf))
    return

  for child of ast_node.children:
    CompileToBytecode(child)
  bytecode.push(CompileOp(ast_node.op))
```

> It's funny: whenever I say I'm going to write some pseudocode, what comes out
> is valid Python.

## Evaluating leaf nodes

Let's examine this pseudocode. First off, note that there's a distinction
between "leaf" and "internal" nodes – the blue and green nodes in our diagram
above, respectively. Leaf nodes are primitive expressions that don't take any
stack inputs. $a is a leaf node, because pushing $a on the stack is
accomplished with the CGetL bytecode:

```
CGetL <Local ID>: push the local with ID <Local ID> onto the stack
```

CGetL does take an input—the ID of the local to push—but this input is a
constant, so it's **part of the bytecode**, not a stack input! In the compilers
world, we often refer to the constant inputs of a low-level instruction as
"immediates". CGetL is a bytecode that takes one immediate - a local ID - and
no stack inputs, and pushes that local onto the stack.

The constant expressions 2 and "x" can similarly be evaluated by bytecodes with
no stack inputs. Again, these bytecodes have immediates - the constant integer
and the constant string to push, respectively. We have:

```
Int <constant int>: push this integer onto the stack
```

and:

```
String <constant string>: push this string onto the stack
```

These three bytecodes - CGetL, Int, and String - are sufficient to cover all
the leaf nodes in our example. Now, let's move onto the internal nodes. The
second half of our CompileToBytecode deals with these nodes.

## Evaluating internal nodes

We've already seen an example of an internal nodes in the previous lesson: the
"Add" bytecode. This bytecode pops its two inputs from the stack, does
"whatever + does in Hack" for those two inputs, and pushes the result into the
stack. To evaluate `(expr_1) + (expr_2)` it suffices to recursively evaluate
the two expressions (putting each result on the stack), then execute an "Add"
bytecode.

"Add" and "Mul" are simple examples of internal nodes, in that they consume a
fixed number of stack inputs and take no immediates. Other nodes introduce a
few wrinkles to this basic pattern:

1. Some nodes take immediates. For example, "IsTypeC" (used to evaluate `(expr)
   is TYPE`) takes one input—the expression—from the stack, but takes the type
   as the immediate.
2. Some nodes take a variable number of arguments from the stack. For example,
   "NewVec <constant int>" (used to evaluate `vec[(expr_1), (expr_2), ...
   (expr_N)]`) takes the number of inputs `N` as an immediate, pops `N` values
   from the stack, and pushes the vec constructed from those values.
3. Some nodes take additional, hidden stack inputs in addition to the syntactic
   inputs from the original Hack source.

We'll examine a function call node—the "Call" node in our example above—which
includes of the complexity above. There are several bytecodes that deal with
function calls in Hack. We support calls to free functions, instance methods,
and class methods, with static and dynamic versions of each, so we need at
least six function call bytecodes. There are several immediates used by all of
these cases, so we package them all up into "FCallArgs":

```
struct FCallArgs {
  uint32_t numInputs;
  uint32_t numOutputs;
  // Other fields for async calls, inout args, coeffects, etc.
};
```

The two most important bits in this struct are the "numInputs" and "numOutputs"
fields, which tell us how many arguments the call bytecode will pop from the
stack, and how many return values it will push. (We model functions with inout
arguments as pushing those results as additional return values, but for any
"normal" function call without such arguments, numOutputs will equal 1.)

The remaining immediates are specific to each type of function call. They
identify which function is being called. Hopefully, the fact that we have
multiple bytecodes for calls makes sense, because we must take different inputs
to identify the "receiver" of free functions, instance methods, and class
methods:

* A free function is identified by its string name.
* An instance method is identified by an object to call the method on and the name of the function to call.
* A class method is identified by a class pointer to call the method on and the name of the function to call.

The bytecode for a static call to a free function, FCallFuncD, takes two
immediates: FCallArgs, and a constant string function name. It has the
following signature:

```
FCallFuncD <FCallArgs> <constant string>:
  Call the function with the given string name, popping FCallArgs.numInputs arguments
  from the stack and pushing FCallArgs.numOutputs return values to it.
```

The final wrinkle for these function calls is that they require—for now!—two
stack inputs before the syntactic arguments to the function. These inputs only
serve to pad the stack in memory with enough space for a [machine-level
function call frame](https://en.wikipedia.org/wiki/Call_stack#Structure) (also
called an "activation record" or "ActRec").

> The fact that a call takes two hidden stack inputs is contingent on the
> current size, in bytes, of [HHVM's in-memory representation of an ActRec](../../runtime/vm/act-rec.h).
> In particular, stack slots occupy 16 bytes in memory—each one is a
> TypedValue!—and an ActRec occupies 32 bytes. It's rare for Hack bytecode to
> be sensitive to low-level implementation details, but in this case,
> unfortunately, it is.

To handle these hidden inputs, we have to modify our pseudocode above to
account for them in its "children" loop. In particular, for any call bytecode,
the hidden inputs are pushed first, before the syntactic arguments.

## Putting it together

Let's run our pseudocode on the AST above. Here's the execution trace, showing the recursive calls, in order:

1. Compile(Add):
   1. Compile(Mul):
      1. Compile(Leaf $a) => **CGetL $a**
      2. Compile(Call):
         1. Push ActRec padding => **2x NullUninit**
         2. Compile(Leaf $b) => **CGetL $b**
         3. Compile(Leaf 2) => **Int 2**
         4. Compile(Leaf 'x') => **String "x"**
         5. Push the op => **FCallFuncD {3, 1} "f"**
      3. Push the op => **Mul**
   2. Compile(Leaf $c) => **CGetL $c**
   3. Push the op => **Add**

Remember that the algorithm produces a linear bytecode sequence via this
in-order tree traversal. Here's the result. Try stepping through it, and verify
that it computes the result of `$a * f($b, 2, 'x') + $c` and leaves it on the
stack!

```
CGetL $a
  NullUninit
  NullUninit
  CGetL $b
  Int 2
  String "x"
  FCallFuncD {3, 1} "f"
  Mul
  CGetL $c
  Add
```

---

## Step 1: Examining basic expressions

In this section, we're going to look at the actual bytecode that HHVM generates
for different cases. There are a few hundred distinct bytecode operations to
keep track of. Luckily, these operations are all documented in a file:

* [hphp/doc/bytecode.specification](../bytecode.specification)

Let's start by double-checking our prediction for the example above. Do so by
creating Hack script that evaluates the given expression and using the
"Eval.DumpHhas=1" debug flag. Put the following code into `~/php/bytecode.php`:

```
<?hh

function test($a, $b, $c) {
  return $a * f($b, 2, 'x') + $c;
}
```
Then run this code with:

```
hphp/tools/hhvm_wrapper.php --hdf Eval.DumpHhas=1 ~/php/bytecode.php | grep -v srcloc
```

(The last part of this command filters out "srcloc" annotations in the
generated bytecode, which attribute bytecode back a file and line in the Hack
source.) If we run this command on our test file, we'll get a compilation of
"test" which is basically the bytecode needed to compute our example
expression. The only additional bytecode is a RetC, which turns an expression
into a return statement: it pops one element off the stack and returns that
value to the caller. Here's a (slightly edited form) of what I get:

```
.function{} (3,5) <"" N > test($a, $b, $c) {
  NullUninit
  NullUninit
  CGetL $b;"b"
  Int 2
  String "x"
  FCallFuncD ... 3 1 ... "f"
  CGetL2 $a;"a"
  Mul
  CGetL $c;"c"
  Add
  RetC
}
```

This bytecode is quite close to our prediction above. There are a few more
components to the FCallArgs struct than what we showed, but that's expected;
Hack supports a variety of special ways to call functions. A bigger difference
is that $a is pushed onto the stack *after* the function call result, and with
a different op—CGetL2, instead of CGetL.

> Wait, what the heck? Shouldn't HHVM evaluate these expressions in order? A
> compiler may re-order expression evaluations as long as doing so has no
> observable effect. For example, we generally can't re-order function calls,
> because function calls could have side effects like writing to the heap,
> throwing errors, or doing IO. Pushing a local onto the stack is side-effect
> free, so in this case, this rewrite is safe.

We should check that this bytecode sequence has the same behavior as the one we
predicted. To do so, we need to understand the semantics of CGetL2. There are a
few ways to find out what a bytecode operation does. You can read:

1. Its docs, by searching for CGetL2 in [bytecode.specification](../bytecode.specification).
2. Its implementation in the interpreter, by searching for "iop$NAME" (i.e. iopCGetL2) in [bytecode.cpp](../../runtime/vm/bytecode.cpp).
3. Its implementation in the JIT, by searching for "emit$NAME" (i.e. emitCGetL2) in the [JIT directory](../../vm/jit).

Here are the docs for CGetL2. (We skip the part about throwing errors, which will not happen in our example.)

```
Get local as cell. If the local variable given by %1 is defined, this
instruction gets the value of the local variable, pushes it onto the stack as
a cell, and then pushes $1 onto the stack.
```

In this explanation, `%1` refers to the first immediate—that is, the local
ID—and `$1` refers to the top element of the stack. Basically, the docs are
saying that CGetL2 pops one element off the stack, pushes the local, then
pushes the popped element on top of it. The net result is that doing CGetL2
after evaluating an expression is equivalent to doing CGetL before evaluating
it.

---

## Step 2: Examining more complex expressions

Let's try out a few more expressions with our setup from above. Try this version:

```
<?hh
class C {}

function test_vec() {
  return vec[17, new C()];
}

function test_dict($x) {
  return dict['a' => 17, 'b' => $x, 'c' => 'd'];
}

function test_concat($x) {
  return 'prefix'.(string)$x.'suffix';
}
```

Here's what I get for these examples. Make sure these outputs make sense. Other
than the complex logic needed to implement the "new C()" syntax, these outputs
are an exact match for what our pseudocode would predict.

```
.function test_vec() {
  Int 17
  NewObjD "C"
  Dup
  NullUninit
  FCallCtor ... {0, 1} ...
  PopC
  LockObj
  NewVec 2
  RetC
}

.function test_dict() {
  Int 17
  CGetL $x
  String "d"
  NewStructDict <"a" "b" "c">
  RetC
}

.function test_concat($x) {
  String "prefix"
  CGetL $x
  CastString
  Concat
  String "suffix"
  Concat
  RetC
}
```

These examples show us a couple of new bytecodes used to implement these basic
elements of Hack syntax:

1. `NewVec 2` pops that two elements from the stack, appends them to a new vec,
   and pushes that vec.
2. "new C()" expands to several bytecodes - from `NewObjD "C"` to `LockObj` -
   that split up the operations of allocating the object and calling the
   constructor. That makes sense, because calling the constructor may require
   evaluating further expressions (the constructor arguments).
3. `NewStructDict <"a" "b" "c">` is similar to NewVec, in that it's a bytecode
   with a variable number of inputs. It takes its values from the stack, but
   the keys are a vector of string immediates.
4. Concatenation happens two inputs as a time - at least, if done with `Concat`
   bytecodes.

---

## Lesson summary

* Hack bytecodes operate on a stack machine. Most bytecodes pop inputs from the
  stack and push an output to it.
* We can compile an expression to bytecode recursively, by compiling its
  subexpressions, then appending a bytecode that applies the top-level
  expression transformation to those inputs.
* Bytecodes take "immediates" - constant arguments that are part of the
  instruction stream - as well as stack inputs.
* Because Hack is a high-level language, it includes bytecodes for constructing
  and operating on complex data structures like objects, strings, vecs, dicts,
  and keysets. [You can see a full list of bytecodes here.](../bytecode.specification)

---

## Exercises

1. Search bytecode.specification for other bytecodes related to string concatenation.
   1. Is there an alternative bytecode sequence that we could use to implement `test_concat`?
   2. Which bytecode sequence would you expect to be faster? Why? (Reading the interpreter may help in this case.)

2. Rewrite `test_dict` from our last example as follows:

```
function test_dict($x, $y, $z) {
  return dict['a' => 17, 'b' => $x, 'c' => 'd', $y => $z];
}
```

   1. How do we implement this modified dict constructor in bytecode?
   2. Are there performance differences between this approach and NewStructDict?
   3. Can you provide an alternative bytecode sequence for this expression that could improve its performance?

Investigate how HHVM handles "member operations"—basically, read/write access
into array elements or object properties. Take a look at the bytecode for these
functions:

```
function prop($x) {
  $x->y = 17;
}

function elem($x) {
  $x['y'] = 17;
}

function nested_one_level($x) {
  $x['y']->z = 17;
}

function nested_two_levels($x) {
  $x['y']->z[] = 17;
}
```

1. How are we handling these sequences of operations in a compositional way?
2. Read the relevant bytecode docs in bytecode.specification, and write Hack
   functions that use the bytecodes BaseH, BaseSC, SetOpM, and UnsetM. Confirm
   with the Eval.DumpHhas output.
3. What changes if we replace these mutating ops with "read-only" member-ops
   like `return $x['y']->z[17];`
4. What state do we need to keep in between steps of a nested member-op write
   sequence? Take a look at [struct MInstrState in this file](../../hhbbc/interp-state.h)
   to confirm your guess.
