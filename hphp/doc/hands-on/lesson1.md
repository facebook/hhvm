# Lesson 1: Understanding Hack runtime types

## Lesson goals

* Learn about the most common user-visible types in Hack.
* See how HHVM uses a "TypedValue" to represent any Hack value.
* Modify the interpreter to treat "+" as the operator for string concatenation!

---

## Step 0: Pull, update, and re-compile HHVM

If you've been following along, you should have [set up your environment](lesson0.md)
in the prior lesson. If not, do so now.

Since it takes quite a while for HHVM to compile, you should generally kick
off a build whenever you plan to edit its code, before making the edits. We're
going to modify the interpreter in this lesson, so start a build now.

---

## Step 1: User-visible Hack types

Hack is a dynamically-typed language. Most Hack values will be one of the
following types:

* null
* bool
* int
* float
* string
* vec
* dict
* keyset
* object

Let's write and execute a small Hack program that uses these types. Save the
following as ~/php/runtime-types.php:

```
<?hh

class C { function __construct() {} }

function show($x) {
  $repr = json_encode($x);
  if (!$repr) $repr = '<unserializable>';
  print('$x is a '.gettype($x).': '.$repr."\n");
}

<<__EntryPoint>>
function main() {
  show(null);
  show(true);
  show(17);
  show(0.5);
  show('hello');
  show(vec[true, 17]);
  show(dict['a' => true, 'b' => 17]);
  show(keyset[17, 'hello']);
  show(new C());
}
```

As always, we'll run this code with HHVM-wrapper:


```
hphp/tools/hhvm_wrapper.php ~/php/runtime-types.php
```

Now, let's try to see if there are any other types. Modify the file to call
"show" on the following additional values:

```
  show(shape('a' => true, 'b' => 17)); // A shape
  show($x ==> $x * $x);                // A closure
  show(show<>);                        // A function pointer
```

Which of these values have new runtime types?

At this point, you may be confused. Hack - that is, the "hh" type checker -
treats shapes and dicts as different types, but as the above examples show,
they have the same type at runtime. To explain what's going on, we need to look
at how this runtime type system differs from the Hack type system.

---

## Runtime types vs. the Hack type system

So far, we've been talking about the type of a value at runtime. That's related
to, but NOT the same as, the type of the value in the Hack type system! Here's
why a runtime type and a type system's type annotation are different:

* A type annotation constrains what a value **could be**, but a runtime type
  tells us what the value **actually is**.
* A type annotation can include union types, like Hack's "arraykey" (int | string)
  "num" (int | float), or even "mixed" (union of everything!) but at runtime,
  any given value must have some specific runtime type.

Let's consider an example. If a parameter $x is typed as an arraykey, then at
runtime, any given value for $x is EITHER an int OR a string. It doesn't make
any sense for it to be both! At the level of the type system, though, we're
just putting constraints on $x, so "arraykey" does make sense as a type
annotation.

To put it yet another way: the "mixed" type annotation means "no constraint".
**No value is "mixed" at runtime!**

Finally, there's an important fact that's specific to the Hack type system: it
is unsound. That means that there's no guarantee that a given Hack type
constraint for a value will actually match the value's type at runtime. We can
use the Hack type system to identify likely bugs, but we cannot assume that all
of its type annotations are correct. Later on, we'll see that HHVM's JIT
includes an alternate, sound type system, and that this type system is
essential to generating efficient machine code!

> A "sound" type system provides some kind of guarantee about runtime behavior.
Typically, the guarantee is that if the whole program type-checks, then when it
is executed, the runtime type of every value will match its type annotation.
One reason that the Hack type system is unsound is that it includes HH_FIXME
and UNSAFE_CAST as "escape hatches" that allow you to ignore type errors. There
are other reasons, too.

For now, we're going to completely ignore Hack's type system. Type annotations
are optional; HHVM can execute Hack code without any type annotations. But as
we've learned, HHVM still tracks the runtime type of the values it operates on!

---

## Step 1: Representing Hack values

At runtime, HHVM needs a way to represent a Hack value that can be any of
Hack's dozen-or-so runtime types.

We use the classic "C-style" solution to this problem: a tagged union.

Let's consider this idea in C-like pseudocode:

```
enum DataType : uint8_t {
  KindOfNull = 0,
  KindOfBool = 1,
  KindOfInt64 = 2,
  ...
  KindOfObject = 8
};

struct Value {
  union {
    // Null doesn't need a value
    bool m_bool;
    int64_t m_int64;
    ...
    ObjectData* m_obj;
  };
};

struct TypedValue {
  Value m_data;
  DataType m_type;
};
```

Basically, this code says that every Hack value is represented as a TypedValue
struct. This struct has two fields: a "tag" byte called DataType, which tells
us what kind of value the TypedValue contains, and the Value, which contains
*overlapping* storage for each of the different kinds of values. (The "union"
keyword means: "the following fields occupy the same location in memory".)

The first thing we should note about this kind of code is: **this kind of C++
data structure is not safe.**

If we read or write to m_data using the wrong interpretation of its union
field, we might accidentally read and use an int64_t value as an ObjectData*
pointer. In doing so, we can arbitrarily corrupt the heap memory of our
runtime!

> Folks like to say that bugs like this one could result in [nasal
> demons](http://catb.org/jargon/html/N/nasal-demons.html), but in practice,
> the most common outcome is that we produce incorrect results for some Hack
> functions, then segfault soon afterwards =)

On the HHVM team, we put up with unsafe practices like the above because it's
the easiest way to express the low-level behavior that we want the machine to
execute. JIT compilation is inherently unsafe, since any bug in the compiled
output is going to produce incorrect behavior that's worse than pretty much any
bug in regular C++ code.

Now, let's look at our actual implementation of this idea. It appears in two files:

* [runtime/base/datatype.h](../../runtime/base/datatype.h)
* [runtime/base/typed-value.h](../../runtime/base/typed-value.h)

Take a look through these files. Do you see the "DataType" enum? What about
"struct TypedValue", in the latter file?

---

## Step 2: Operating on Hack values

Now, we're going to look at the simplest possible way to operate on these Hack
values.

As we saw in the previous lesson, HHVM can execute an "Add" bytecode to do
"whatever Hack + does" for two arbitrary Hack values! Like all operations in
HHVM, this "Add" bytecode is implemented in both the interpreter and the JIT.

In the interpreter, the implementation of "Add" is as simple as it gets:

1. We pop two inputs from the stack. (Our bytecode assumes that we're using a "stack machine".)
2. We check the input types. Today, "Add" only works on "numeric" - int or float - inputs. We throw on other types.
3. We switch over each of the valid input types, and implement the addition logic for each one.
4. We push the resulting TypedValue output back onto the stack.

Now, Hack already has a string concatenation operator: ".". But perhaps you may
have wondered: why not use "+" for string concatenation in Hack, like other
languages like JavaScript and Python support? Let's make it happen!

The logic for the interpreter is in the following file:

* [runtime/vm/bytecode.cpp](../../runtime/vm/bytecode.cpp)

Search in this file for a function called "iopAdd". This function implements
the logic above. Read it, and see how it works. You should find that it uses a
generic helper to implement the "pop 2 inputs from the stack and push 1 output"
logic. This generic helper takes the binary operation to perform on the two
inputs as a parameter, and for "iopAdd", that operation is "tvAdd", which is in
this file:

* [runtime/base/tv-arith.cpp](../..//runtime/base/tv-arith.cpp)

Once you've read enough of these functions to have a basic understanding,
modify them so that tvAdd also concatenates two string inputs! Use the
definition of the TypedValue, above, and the API for strings in this file to
help you out:

* [runtime/base/string-data.h](../../runtime/base/string-data.h)

After making these changes, recompile HHVM. Remember that your implementation
ONLY modifies the behavior of the interpreter - NOT the JIT!  That means that
HHVM will have two different behaviors based on whether we are interpreting or
compiling a piece of Hack code. That's not good, but for now, we can still test
this interpretation by running HHVM with the JIT disabled.  Put the following
test case into ~/php/concat.php:

```
<?hh

function add($x, $y) {
  return $x + $y;
}

<<__EntryPoint>>
function main() {
  var_dump(add(17, 34));
  var_dump(add(17.0, 34.0));
  var_dump(add("17", "34"));
}
```

Then run:

```
hphp/tools/hhvm_wrapper.php -i ~/php/concat.php
```

If you've done this step, you should see the results 51, 51.0, and "1734"
printed to stdout!

---

## Lesson summary

- Hack is a dynamically-typed language. At runtime, a Hack value has some
  specific runtime type.
- A "type system" constrains what types a value may have. A type system can be
  "sound", meaning that its claims about types are guaranteed to hold for
  values at runtime.
- The Hack type-checker's type system is unsound. HHVM uses a separate, sound
  type system for JIT compilation.
- HHVM represents Hack values as a tagged union: a 1-byte DataType enum, which
  can be used to interpret an 8-byte Value. The Value may be a pointer (e.g.
  strings, objects), an int or bool value, or unused (e.g. for null).
- In the interpreter, we implement Hack bytecode by doing casework on the value
  of this DataType.

---

## Exercises

1. What "secret" DataTypes exist in HHVM but are invisible to Hack? What do you think "KindOfUninit" is?
2. Extend your modified tvAdd so that, if at least one of the two inputs is a string, it uses the logic for Hack "string casts" (the syntax: `(string)$x`) to cast the other input to a string, then concatenates the results.
   1. String casts have an associated bytecode, "CastString". That means that you can search bytecode.cpp for "iopCastString" to see how string cast is implemented, and share that logic.
   2. Test your work by modifying the file to use "add" to concatenate a string and an int, or a string and a float. Does it work? If so, congratulations: **you have just implemented dime-store JavaScript.**
3. Enable the JIT again. How do the results of the "concat.php" file change?
   1. You may be surprised to see that even with the JIT enabled, we still use "+" for string concat - or do we?
   2. Enable the JIT and also enable the "TRACE=printir:1" debug output from the previous lesson. Look at how many times we compile "add", and for which types. Does the string version of "add" explain your results?
4. In C++, we can ask for the "sizeof" a given struct, in bytes. Take a look at this example, where we use static_assert (checked at compile time) to check that different types have the sizes we expect: https://godbolt.org/z/bP4xTPMPq
   1. Why is a DataType 1 byte?
   2. Why is a Value 8 bytes? Does it matter that a bool can fit in 1 byte?
   3. How large is a TypedValue? Add static_assert(sizeof(TypedValue) == ...); to confirm.
   4. Why is a TypedValue that large? (Hint: read [http://www.catb.org/esr/structure-packing/](http://www.catb.org/esr/structure-packing/))
