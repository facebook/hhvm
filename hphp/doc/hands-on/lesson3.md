## Lesson goals:

* Explore a unique feature of the PHP and Hack languages: value-type strings and array-likes.
* Learn about how HHVM uses reference counting to efficiently implement value types and to reclaim memory.
* See how certain Hack bytecodes result in refcounting operations.
* See how HHVM uses an ahead-of-time compilation step to reduce unnecessary refcounting.
* Make and test a small attempt at a performance optimization.

---

## Step 0: Mysterious missing mutations

Save the following Hack code in `~/php/refcounting.php`:

```
<?hh
class C { function __construct(public int $x) {} }
function mutate_int(int $x) {
  $x = 34;
}
function mutate_object(C $c) {
  $c->x = 34;
}
function mutate_dict(dict<string, int> $d) {
  $d['x'] = 34;
}
<<__EntryPoint>>
function main() {
  $x = 17;
  mutate_int($x);
  var_dump($x);
  $c = new C(17);
  mutate_object($c);
  var_dump($c);
  $d = dict['x' => 17];
  mutate_dict($d);
  var_dump($d);
}
```

What would you expect to see if you ran this code? Make a guess before reading on.

...

...

...

...

...

...

...

...

...

...

...

...

...

...

...

...

Okay, let's run the file. I assume you already have HHVM compiled at this point! Run:

```
hphp/tools/hhvm_wrapper.php ~/php/refcounting.php
```

Here's what you should see as output:

```
int(17)
object(C) (1) {
  ["x"]=>
  int(34)
}
dict(1) {
  ["x"]=>
  int(17)
}
```

What is going on here? The caller `main()` sees the modification made to `$c`
in `mutate_object()`, but it doesn't see the modification made to `$d` in
`mutate_dict()`. Clearly, HHVM is broken. It's a compiler bug! Shut it down!

...no. This result is correct. In PHP and Hack, objects are "passed by
reference" or are "reference types", but strings, vecs, dicts, and keysets are
"passed by value" or are "value types". Primitives like ints, floats, booleans,
and the value "null" are also value types.

> Vecs, dicts, and keysets share many similarities. They're heap-allocated
containers of varying size with value-type semantics. We use the term
"array-likes" throughout our codebase to refer to this trio of types.

When we pass an object as a function argument, we're passing a reference to a
unique value. Mutations to that object are visible to anyone who has a
reference to it. On the other hand, **when we pass a value type as a function
argument, the caller's copy and the callee's copy of that value are logically
distinct**. Mutations made to a value type inside a function are not visible
outside it.

Now, it's not surprising that primitives like ints are value types. Indeed, any
other implementation would be quite inefficient!  [As we saw in Lesson 1](lesson1.md),
ints, floats, and booleans are stored directly in the TypedValue—they aren't
hidden in heap-memory behind a pointer. When we push `$x` onto the stack to
call `mutate_int()`, the value on the stack is independent of the value in the
local.

It's also not surprising that objects are reference types. Again, from Lesson
1, we saw that objects are heap-allocated. All that we store in a TypedValue is
a pointer to this heap memory; the object's class and its properties are stored
in the heap. When we push the object `$c` onto the stack, we're copying the
pointer, not the object! As a result, when we write through this pointer to
modify the object in `mutate_object()`, the effect is visible in main! (If
`mutate_object()` first replaced `$c` with a new object - i.e. `$c = new C()`
- then modifications to this value would not affect the original one.)

The surprising bit here is that in PHP and Hack, strings and array-likes are
**heap-allocated value types**. Even though these values are represented as a
pointer to the heap, when we pass them to a function, the two pointers act like
distinct values. Take a moment to think about how you might implement this
behavior. One straightforward approach is to copy these values when we pass
them as function arguments, but that would be a performance disaster. For
example, it would turn Shapes::idx from a simple O(1) hash table lookup into an
O(n) dict copy! We need a better way.

---

## Enter refcounting

HHVM implements efficient heap-allocated value types using reference counting.
**In fact, all heap-allocated values are refcounted, regardless of whether
they're reference types or value types.** Refcounting serves two purposes:

1. It's a way to eagerly free heap-allocated values. We also use a
   [mark-sweep garbage collector](https://en.wikipedia.org/wiki/Tracing_garbage_collection#Na%C3%AFve_mark-and-sweep)
   to handle cycles.
2. It enables [copy-on-write (COW)](https://en.wikipedia.org/wiki/Copy-on-write)
   for strings and array-likes, allowing us to pass them as arguments in O(1) time.

The first purpose applies to objects, strings, and array-likes. Like Java and
Python, and unlike C++, Hack automatically manages memory for its users. It
frees heap-allocated values that are no longer used. Refcounting is one
approach to automated memory management: if a heap-allocated value has no
references, we can free its memory. The second purpose only applies to
heap-allocated value types, i.e. strings and array-likes.

> Refcounting is not a complete solution to memory management!
> Read ["Moon instructs a student"](http://www.catb.org/jargon/html/koans.html) to see why.

Let's look at how we implement refcounting. In our code, every heap-allocated
value starts with an 8-byte header, the C++ struct HeapObject. This struct is
defined in [the file header-kind.h](../../runtime/base/header-kind.h).
(Don't ask why. The reason has been lost to the mists of time.) Thus, we can
say that even though strings and array-likes are not Hack objects, all
heap-allocated values are HeapObjects.

> Be careful with the terminology! A HeapObject is any heap-allocated value. It
can be a reference type—e.g. an object—or a value type—e.g. a string. We will
always use "object" to refer to Hack objects, and "HeapObject" to refer to the
union of Hack objects, strings, and array-likes.

Every HeapObject tracks a **reference count**: a count of the number of
references to that value that are currently live. These references might be
locals or stack elements. If a HeapObject appears in a vec, the vec holds a
refcount on the HeapObject. If a HeapObject is stored to an object's property,
then the object holds a refcount to the HeapObject. A HeapObject's refcount
stores the number of distinct places that have a pointer to that HeapObject.

> That's not actually true! There are some cases where HHVM skips decrementing
refcounts, making the refcounts an overestimate. The real constraint is that
**a HeapObject's refcount is an upper bound on the number of distinct places
that have a pointer to that HeapObject**. But let's ignore this subtlety.

Every time we copy a HeapObject pointer—for example, if we push a copy of a
HeapObject local onto the stack—we increment the HeapObject's refcount
("inc-ref"). Every time we overwrite or let go of a HeapObject pointer—for
example, if we overwrite a HeapObject local, or if we pop one off the stack—we
decrement its reference count ("dec-ref").

Using refcounting for memory management is simple. Every time we dec-ref a
HeapObject, we check if its refcount is 0. If it is, then we  know that that
object will never be used again, and we can free its associated memory.

Using refcounting to implement efficient heap-allocated value types is a bit
more involved. Whenever we modify a value-type HeapObject, such as a dict, we
use a strategy called copy-on-write:

* If the dict has refcount == 1, we modify it in place. No one besides us has a
  pointer to the dict, so we're still maintaining value-type semantics, despite
  doing O(1) in-place edits.
* If the dict has refcount != 1, we copy it (to get a copy with refcount == 1)
  and the modify the copy. We dec-ref the old value since we've replaced our
  pointer to it with a pointer to the copy.

---

## Refcounting and bytecode

The stack machine semantics of a Hack bytecode imply refcounting bookkeeping
that we have to do to implement it.

We can figure out how to do this refcounting by asking ourselves a question: if
the inputs to a given bytecode are refcounted HeapObjects, how many pointers to
those objects exist after the operation is complete?

Let's start with the simplest case, a CGetL bytecode. From
[bytecode.specification](../bytecode.specification):

```
CGetL <local variable id>    []  ->  [C]

  Get local as cell. If the local variable given by %1 is defined, this instruction
  gets its value and pushes it onto the stack. If the local variable is not defined,
  this instruction raises a warning and pushes null onto the stack.
```

This bytecode takes a local and pushes it onto the stack. If the local is a
HeapObject, then there is now one additional copy of that heap pointer. **In
addition to pushing a value on the stack, CGetL must inc-ref it.**

PushL is a related bytecode that "pushes" a value onto a stack instead of
"getting" it. The difference is that PushL both gets the value of the local on
the stack and unsets the local itself (i.e. sets the local to Uninit, which is
Hack's flavor of "undefined").

```
PushL <local variable id>    []  ->  [C]

  Teleport local value to eval stack. The local given by %1 must be defined.
  This instruction pushes the local's value on the stack, then unsets it,
  equivalent to the behavior of a CGetL / UnsetL pair.
```

If the local that we pushed with PushL is a HeapObject, then we've gained
another copy of that heap pointer on the stack, but we've unset the pointer in
the local. Overall, its refcount is unchanged. **Because PushL "moves" a heap
pointer, it does no refcounting.**

Let's look at one more case, an operation that uses a stack value to set a local:

```
PopL <local variable id>    [C]  ->  []

  Teleport value from the stack into a local. This instruction stores the top
  value of the stack $1 to the local %1, then pops this value off of the stack.
```

Let's say the value on top of the stack was a HeapObject. As with PushL, we're
gaining one reference to it (the new value of the local), but we're also losing
one reference to it (the one we pop off the stack). We do not need to inc-ref
or dec-ref the stack value. However, there is refcounting associated with PopL:
if the old value of the local was a HeapObject, we must dec-ref it! **PopL does
not need to inc-ref or dec-ref the stack value that it's storing to a local,
but it must dec-ref the local's old value.**

Now that you've seen these examples, predict the refcounting needed for these bytecodes:

1. **UnsetL:** "unsets" a local by setting its value to Uninit.
2. **NewVec:** pops n values off the stack and creates a vec containing those elements.
3. **RetC:** pops and returns the top value of the stack, cleaning up the function's frame.
4. **Concat:** pops two values off the stack; if they're not both strings, it throws, else, it concatenates them and pushes the result.

...

...

...

...

...

...

...

...

...

...

...

...

...

...

...

...

If UnsetL is operating on a refcounted local, it must dec-ref the old value.
**UnsetL dec-refs the local's old value.**

NewVec can "move" any HeapObjects it's taking as stack inputs. A stack slot
used to refer to them; now, a vec element refers to them; the overall
refcounting effect is neutral. However, NewVec allocates a new, refcounted vec
and pushes a pointer to it onto the stack. This new vec should have a refcount
equal to 1. **Constructors like NewVec create a new HeapObject with a refcount
of 1.**

RetC signifies a function return. It "frees" the memory associated with the
function's frame. (Frames are usually stack-allocated, so freeing the memory is
just decrementing the stack offset.) Functions may hold references to
HeapObjects in their locals, so before we "free" the frame, we must dec-ref
these values. **RetC dec-refs the function's locals.**

Let's assume Concat is operating on two refcounted strings. Concat should
allocate a new string, so it's acting kind of like a constructor; the new
string will have a refcount of 1. Concat also pops two strings off the stack,
so it must dec-ref those. **In general, stack-only bytecodes like Concat
dec-ref their inputs and either allocate a new output with refcount 1, or
inc-ref an existing value to push it as an output.**

---

## Step 1. Visualizing refcounting

When we run HHVM in interpreted mode, we can turn on debug tracing to see refcounting working as above. Put the following code into `~/php/refcounting.php`:

```
<?hh
class LinkedList { function __construct(public $next = null) {} }
<<__EntryPoint>>
function main() {
  $x = new LinkedList();
  $y = new LinkedList($x);
  $z = $y;
  $y = 17;
  var_dump($z);
  $z = 34;
}
```

Then, run this code in interpreter mode with refcounting debug enabled:

```
TRACE=bcinterp:9 hphp/tools/hhvm_wrapper.php -i ~/php/refcounting.php
```

The resulting output shows the types and refcounts of all locals and stack values at each bytecode. For example, just prior to the final RetC that ends the function main(), we have:

```
dispatch:                    __
dispatch:                    || === Stack at /home/kshaunak/php/refcounting.php:13 func main ===
dispatch:                    || {func:main,callOff:0,this:0x0}<C:0x7f06b903e320:c(1):Object(LinkedList) C:0x11 C:0x22> C:Null
dispatch:                    \/
dispatch: 71: RetC
```

This output says that in function `main()`, local 0 (which is `$x`) contains a
LinkedList with refcount 1 at heap address 0x7f06b903e320. The other two
locals, `$y` and `$z`, are the integers 0x11 and 0x22, respectively, because we
assigned to them earlier. There's one value on the stack, a null, which is the
value that the RetC will return.

Look through this output and see how LinkedList refcounts change with CGetL /
SetL bytecodes. Confirm for yourself that the refcounting semantics match our
predictions above. What is the maximum refcount we see for any object in this
function? Which object has that refcount, which bytecode does it occur at, and
where are all of the references to it?

---

## Step 2: Static strings and array-likes

Strings and array-likes are value types. A bit of Hack code that has a
reference to one of these values can guarantee that no other Hack code can edit
that value. HHVM implements heap-allocated value types by storing a refcount in
their heap data, and blocking mutation for value types with a refcount that is
not equal to 1.

This constraint allows HHVM to make a critical optimization: placing strings
and array-likes in shared memory. Most Hack objects are thread-local. HHVM can
run multiple requests concurrently, and a given object is only referenced on a
single thread: the thread that allocated it. But this approach would be a
disaster for commonly used values like the empty vec or empty dict! Instead,
we'd like to be able to share these values between requests. Multiple requests
would all have concurrent access to a single empty vec and dict.

Here's an approach to sharing these values that does NOT work:

* Store a single, empty vec in a process global with an initial refcount of 1.
* When we retrieve this vec on a request, we inc-ref it, so it starts with a
  refcount of at least 2. We inc-ref and dec-ref it as normal when executing
  bytecode for each request.
* Because the minimum refcount of this vec, as seen from any Hack code, is 2,
  no one will ever modify it.

This approach seems to work. Whenever Hack code is manipulating this global
vec, it will have a refcount of at least 2 (one held by the global itself, and
one held by the value in the request), so it can't be modified. The bug here is
subtle: **modifying any value in shared memory introduces a race condition,
unless done with atomic memory operations**. The bug here is that we can't
actually inc-ref or dec-ref any values in shared memory!

Here's an example of a race condition that can occur when inc-ref-ing the
shared vec:

1. Thread A loads its initial refcount X.
2. Thread B loads its initial refcount X.
3. Thread A writes back the incremented refcount X + 1.
4. Thread B writes back the incremented refcount X + 1.

Even though threads A and B have created two new references to the shared vec,
the refcount has only increased by 1. At some point, when these threads dec-ref
the pointers they hold, they can dec-ref the value to 0 and "free" the array,
causing HHVM to crash!

Instead, here is how HHVM implements shared-memory strings and array-likes:

* The refcount of a string or array-like can be either some positive or
  negative value.
* If the refcount is positive, the HeapObject is thread-local; if it is
  negative, it is in shared memory.
* When doing any inc-ref or dec-ref op on a string or array-like, we first
  check the refcount's sign. If it's negative, we do nothing!

Since concurrent reads on shared memory are safe, this approach doesn't
introduce any race conditions. We can share a single empty vec and empty dict,
and many other constant HeapObjects, between all of our requests! Let's look at
a quick example. HHVM places any "constant value" like a constant string or vec
into shared memory. Put the following code into `~/php/static.php`:

```
<?hh
<<__EntryPoint>>
function main() {
  $x = vec[17, 34];
  var_dump($x);
  $y = 'my constant string';
  var_dump($y);
}
```

Then run it with:

```
TRACE=bcinterp:9 hphp/tools/hhvm_wrapper.php -i ~/php/static.php
```

When we manipulate `$x` and `$y` as locals and as stack values, you should see
a refcount of "c(static)" for them: a sentinel negative refcount value. HHVM's
tvIncRefGen and tvDecRefGen, the generic refcounting helpers, avoid mutating
static strings and array-likes by checking that the refcount is positive before
decrementing it. The refcount of a static string or array-like will never
change!

> You might ask, why don't we use atomic increments or decrement operations
  like "lock-add" to inc-ref and dec-ref all values? Doing that would save a lot
  of instructions: a compare-and-branch on every refcounting op. Unfortunately,
  it would also tank performance.
  [On x86-64, in order to write to any memory, a core must take exclusive access to that memory address](https://en.wikipedia.org/wiki/MESI_protocol)
  (and to the whole cache line containing that address). That means that
  inc-ref-ing a shared value is essentially single-threaded.

As compiler engineers, we should be wary of relying on delicate invariants like
"the refcount of a static value is never mutated". Luckily, we have an ace up
our sleeve to help us enforce such guarantees: the debug-mode build! The
refcounts that are 1 less and 1 greater than the "StaticValue" refcount are
considered to be invalid refcounts. In debug mode, if we ever see a heap object
with a negative refcount that's close to, but not equal to, "StaticValue", we
crash with a nice error message.

> Assertions are a critical tool for testing our code. We check these
  assertions in debug builds, then compile them out in release builds where
  performance is a top priority. In HHVM code, the "assertx" helper checks an
  assertion and prints a stack trace and other diagnostics on failure.
  **Structure your code so that clear invariants hold, then use assertions to
  check them wherever possible!**

---

## Step 3: Refcounting optimizations

As we've seen before, a compiler like HHVM is free to diverge from the
"trivial" implementation of a given bytecode, as long as the logic that HHVM
executes matches the stack machine in any observable behavior. In the previous
lesson, HackC and HHVM used this freedom to re-order some bytecodes, but the
resulting logic was not meaningfully different than the trivial implementation.
Now, we'll look at how we can tweak compilation to gain performance.

**Refcounting is expensive, because it requires accessing heap memory.** Memory
access is one of the most expensive operations on modern CPUs - far more
expensive than doing arithmetic on values in hardware registers. As a result,
saving inc-refs or dec-ref ops is usually a nice win. Furthermore, an inc-ref
and dec-ref on the same value cancel out! The "PushL" bytecode uses this fact
for optimization. Verify for yourself that these bytecode sequences have the
same semantics:

1. CGetL $local; UnsetL $local
2. PushL $local

If we execute the first sequence the "trivial" way, then we'll inc-ref the
local for CGetL (because its value is now in the local AND on the stack), then
immediately dec-ref it (in order to overwrite it with Uninit). Because PushL
does the two ops together, we can cancel these two operations - both when we're
interpreting Push, and when we're JIT-compiling it. HackC does not directly
emit PushL bytecodes, but HHVM comes with an ahead-of-time bytecode optimizer
called HHBBC that does these rewrites. (HHBBC even adds UnsetL ops for locals
that are "dead" - unread by later code in a function - in order to create more
optimizable sequences.)

> Remember that HHVM converts, or "lowers" bytecode to machine code in stages.
It compiles bytecode to intermediate representation (HHIR), then IR to virtual
assembly (vasm), then vasm to machine code. Most optimizing compilers implement
similar optimizations at multiple stages. For example, HHBBC optimizes
bytecode; it replaces bytecode sequences with equivalent sequences with better
refcounting properties. But if we lower bytecode to IR that does an inc-ref
followed by a dec-ref of the same value, we have IR level optimizations (see:
refcount-opts.cpp) that can cancel those ops as well!

Now, let's try to make our own refcounting optimization! Because HHVM has a
garbage collector (GC), we can skip dec-refs without changing the result of any
Hack operations. **Taken to an extreme, we could modify HHVM's DecRef code
generation to return immediately, so dec-refs are a no-op at the machine code
level!** At this point, we'd leak memory to the GC all over the place, but
refcounting would be a lot faster!

> Does this change affect any observable behavior? It turns out that the notion
of "observable" is a bit fuzzy! If we leak this much memory, there's a good
chance the teams monitoring HHVM on live webservers will see a large
increase in memory used. Worse still, the OS's OOM killer (which kills
processes that use too much memory) might kill HHVM servers. Leaking too much
memory is an observable, but leaking a small amount may be okay.

We're going to try a less extreme version of this optimization. The Idx
bytecode takes an array, a key, and a default value as stack inputs. It does
the array access and pushes the result onto the stack, pushing the default
value if the key is not present in the array. Idx must inc-ref the stack output
and dec-ref the stack inputs.

In our codebase, the code that lowers a bytecode $NAME to IR is in a function
called emit$NAME. So, we can find the code to compile Idx by searching for Idx
in the hphp directory. It turns out that it's in
[irgen-builtin.cpp](../../runtime/vm/jit/irgen-builtin.cpp).

1. Modify emitArrayIdx to avoid dec-ref-ing any of the inputs. (It's sufficient to modify the vec and dict cases.)
2. Write a small test file using idx() to confirm that your modification is
   working. You can look at the TRACE=printir:1 output to see that it has fewer
   dec-refs than before (compare the output between your modified HHVM and the
   output from release HHVM). As a shortcut, pass `-b /usr/local/hphpi/bin/hhvm`
   to hhvm_wrapper.php to use release HHVM without recompiling!
3. Kick off local unit tests for your code: `hphp/test/run quick slow`

When you have these steps working, you'll have saved a small amount of
instructions, loads and stores at the cost of a significant memory penalty.
Is this tradeoff an overall CPU win? Only data can tell - but my guess is,
it's a regression! In the next lesson, we'll try to find a performance win.

---

## Lesson summary

* Hack has multiple heap-allocated types: objects, strings, and array-likes
  (vecs, dicts, and keysets).
* Of these types, objects are reference types, while strings and array-likes
  are value types. A mutation to an object is visible to everyone with a
  reference to it. On the other hand, two references to a string behave like
  distinct values.
* HHVM uses refcounting for two purposes: to implement copy-on-write value
  types, and to eagerly free heap values.
* HHVM uses a negative refcount to indicate that strings and array-likes are in
  shared memory. This trick allows us to de-duplicate "static", or constant,
  strings and array-likes across requests.
* Because Hack bytecodes can create new references to heap values, HHVM must do
  inc-refs and dec-refs to implement these bytecodes. For example, we inc-ref the
  stack outputs of a bytecode and dec-ref the stack inputs.

---

## Exercises

1. Remember that HeapObjects include reference types like objects as well as
   value types like strings and array-likes. Why doesn't HHVM support "static"
   (shared-memory) objects?
2. Take a look at HHVM's tvDecRefGen, which does a generic dec-ref of a
   TypedValue. Note that this helper can be used on any TypedValue, including
   non-heap-allocated values like integers.
   1. How many cases does tvDecRefGen have to handle? Confirm by reading the comments in [decref-profile.h](../../runtime/vm/jit/decref-profile.h).
   2. Is it safe to use tvDecRefGen to dec-ref an object?
   3. Is it optimal to use tvDecRefGen to dec-ref an object? What could we do instead?
3. Modify your code for the Idx bytecode to skip the inc-ref of the new value.
   1. Why is the new code unsafe? Why is it okay to skip a dec-ref, but not an inc-ref?
   2. Commit this modified version run the tests with `hphp/test/run quick slow`
   3. If those tests are passing, you can also add other flags to the command:
       a. To test "repo mode", pass "-r" to hphp/test/run. This flag turns on HHBBC.
       b. To test optimized JIT compilation, add "--retranslate-all 2".
       c. Typical HHVM CI will run a battery of tests including these flags.

Here is an hphp/test/run command that includes all of the flags above:

```
hphp/test/run -m jit -r --retranslate-all 2 hphp/test/quick/dict/basic-getters.php
```

Once you can reproduce a failure, fix them by re-adding the inc-refs you
removed and confirm that the test now passes.
