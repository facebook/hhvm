# Introduction

Contexts and capabilities provide a way to control what operations a function may perform. A **capability** is a permission: for example, the permission to do IO, to mutate object properties, or to access global state. A **context** is a named set of capabilities that you attach to a function declaration. The typechecker enforces that a function only performs operations allowed by its capabilities, and that it only calls other functions whose required capabilities are a subset of its own.

## Basic Declarations

A function or method may optionally declare a context list in square brackets after its parameter list:

```hack no-extract
function no_listed_contexts(): void {/* ... */}
function empty_context()[]: void {/* ... */}
function one_context()[write_props]: void {/* ... */}
function many_contexts()[read_globals, write_props]: void {/* ... */}
```

There exists a context named `defaults` that represents the set of capabilities present in a function without an explicit context list. When a function is not annotated with a context list, it implicitly receives a list containing only the `defaults` context.

The above declaration of `no_listed_contexts` is fully equivalent to the following:

```hack
function no_listed_contexts()[defaults]: void {/* ... */}
```

Additionally, the context list may appear in function types:

```hack
function has_fn_args(
  (function (): void) $no_list,
  (function ()[globals, write_props]: void) $list,
  (function ()[]: void) $empty_list,
): void {/* ... */}
```

## Local Operations

Capabilities control what operations a function may perform in its own body. For example, a function without the `WriteProperty` capability cannot mutate object properties, and a function without `IO` cannot use `echo` or `print`:

```hack error
function cannot_do_io()[]: void {
  echo "Hello world\n"; // error: IO capability required
}

class Foo {
  public int $x = 0;
}

function cannot_write_props(Foo $f)[]: void {
  $f->x = 1; // error: WriteProperty capability required
}
```

See [Available Contexts & Capabilities](/hack/contexts-and-capabilities/available-contexts-and-capabilities) for the full list of capabilities and what they gate.

## Interaction of Contextful Functions

In order to invoke a function, the caller must have all of the capabilities required by the callee. A caller may have more capabilities than the callee requires.

```hack
class Counter {
  public static int $count = 0;
}

function pure_fun()[]: void {
  return;
}

function read_count()[read_globals]: void {
  $_ = readonly Counter::$count;
}

function increment_count()[globals]: void {
  Counter::$count++;
  read_count();  // ok: AccessGlobals includes ReadGlobals
  pure_fun();    // ok: pure requires no capabilities
}

function unannotated_fun(): void {
  increment_count(); // ok: defaults includes AccessGlobals
  read_count();      // ok: defaults includes ReadGlobals
  pure_fun();        // ok: pure requires no capabilities
}
```

If the caller lacks a capability the callee requires, the typechecker reports an error:

```hack no-extract
function only_writes_props()[write_props]: void {
  increment_count(); // error: WriteProperty does not include AccessGlobals
}
```

## Dependent Contexts

While the contexts shown above are fixed, Hack also supports **dependent contexts** where a function's context is determined by its arguments or by class constants. These are covered in the [Higher Order Functions](/hack/contexts-and-capabilities/higher-order-functions) and [Context Constants](/hack/contexts-and-capabilities/context-constants) sections.

## Implications for Backwards Compatibility

We may add additional capabilities in the future. As capabilities are specified in terms of what's permitted rather than what is not, the more restrictive your capability annotations are, the more likely it is that future changes will be incompatible with your code. This is especially true for functions that have the empty capability set. This should be considered as a tradeoff against increased confidence in more restricted code.
