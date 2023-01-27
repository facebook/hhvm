**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

Contexts and capabilities provide a way to specify a set of capabilities for a function's implementation and a permission system for its callers. These capabilities may be in terms of what functions may be used in the implementation (e.g. a pure function cannot call non-pure functions), or in terms of other language features (e.g. a pure function can not write properties on `$this`).

Capabilities are permissions or descriptions of a permission. For example, one might consider the ability to do IO or access globals as capabilities. Contexts are a higher level representation of a set of capabilities. A function may be comprised of one or more contexts which represent the set union of the underlying capabilities.

## Defining contexts and capabilities

At present, all declarations of contexts and capabilities live within the typechecker and runtime. There are no plans to change this in the immediate future.

## Basic Declarations

A function or method may optionally choose to list one or more contexts:

```hack no-extract
function no_listed_contexts(): void {/* some fn body */}
function empty_context()[]: void {/* some fn body */}
function one_context()[C]: void {/* some fn body */}
function many_context()[C1, C2, Cn]: void {/* some fn body */}
```

There exists a context named `defaults` that represents the set of capabilities present in a function prior to the introduction of this feature. When a function is not annotated with a context list, it implicitly received a list containing only the default context.

The above declaration of `no_listed_contexts` is fully equivalent to the following:

```hack
function no_listed_contexts()[defaults]: void {/* some fn body */}
```

Additionally, the context list may appear in function types:

```hack no-extract
function has_fn_args(
  (function (): void) $no_list,
  (function ()[io, rand]: void) $list,
  (function ()[]: void) $empty_list,
): void {/* some fn body */}

```

## Interaction of Contextful Functions

In order to invoke a function, one must have access to all capabilities required by the callee. However, the caller may have more capabilities than is required by the callee, in which case simply not all capabilities are "passed" to the callee.

In the following example, assume the existence of a `rand` context representing the capability set `{Rand}`, an `io` context representing the capability set `{IO}`, and that the `defaults` contexts represents the capability set `{Rand, IO}`.

```hack no-extract
/* has {} capability set */
function pure_fun()[]: void {
  return;
}

function rand_int()[rand]: int {
  return HH\Lib\PseudoRandom\int();
}

function rand_fun()[rand]: void {
  pure_fun(); // fine: {} ⊆ {Rand}
  rand_int(); // fine: {Rand} ⊆ {Rand}
}

 // recall that this has the `defaults` context
function unannotated_fun(): void {
  rand_fun(); // fine: {Rand} ⊆ {IO, Rand,}
}
```

## Parameterized Contexts

While most contexts and capabilities represent the binary options of existence and lack thereof, it is also possible for either/both to be parameterized.

In the following example, assume the existence of a `throws<T>` context representing the capability set `{Throws<T>}`. Rather than describing that a function *can* throw, this would describe which classes of exceptions a function may throw. In that scenario, the context would require a parameter representing the exception class: `throws<-T as Exception>`.

```hack no-extract
function throws_foo_exception()[throws<FooException>]: void { // {Throws<FooException>}
  throw new FooException();
}

function throws_bar_exception()[throws<BarException>]: void { // {Throws<BarException>}
  throw new BarException();
}

function throws_foo_or_bar_exception(bool $cond)[
  throws<FooException>, throws<BarException> // {Throws<FooException>, Throws<BarException>}
]: void {
  if ($cond) {
    throws_foo_exception();
  } else {
    throws_bar_exception();
  }
}
```

The above would indicate that throws_foo_or_bar_exception may throw any of the listed exception classes.

# Implications for Backwards Compatibility

We may add additional capabilities in the future. As capabilities are specified in terms of what's permitted rather than what is not, the more restrictive your capability annotations are, the more likely it is that future changes will be incompatible with your code. This is especially true for functions that have the empty capability set. This should be considered as a tradeoff against increased confidence in more restricted code.
