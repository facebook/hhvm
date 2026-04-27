# Available Contexts & Capabilities

The following contexts and capabilities are implemented at present.

## Capabilities

### IO

This gates the ability to use the `echo` and `print` intrinsics within function bodies.

```hack
function does_echo_and_print(): void {
  echo 'like this';
  print 'or like this';
}
```

### WriteProperty

This gates the ability to modify objects within function bodies.

At present, all constructors have the ability to modify `$this`. Note that this does *not* imply that constructors can call functions requiring the `WriteProperty` capability.

```hack
// Valid example

class SomeClass {
  public string $s = '';
  public function modifyThis()[write_props]: void {
    $this->s = 'this applies as well';
  }
}

function can_write_props(SomeClass $sc)[write_props]: void {
  $sc->s = 'like this';
  $sc2 = new SomeClass();
  $sc2->s = 'or like this';
}
```

```hack
// Invalid example

class SomeClass {
  public string $s = '';
  public function modifyThis()[]: void {  // pure (empty context list)
    $this->s = 'this applies as well'; // ERROR
  }
}

function pure_function(SomeClass $sc)[]: void {
  $sc->s = 'like this'; // ERROR
}
```

Hack Collections, being objects, require this capability to use the array access operator in a write context.

```hack
function modify_collection()[write_props]: void {
  $v = Vector {};
  $v[] = 'like this';
  $m = Map {};
  $m['or'] = 'like this';
}
```

### ReadGlobals

This gates the ability to read static variables and globals.
Built-in functions that read from mutable global state will require this capability.

Note that references to static properties from a `read_globals` context must use the `readonly` keyword.

```hack
class HasStatic {
  public static int $count = 0;
}

function read_static()[read_globals]: readonly int {
  return readonly HasStatic::$count;
}
```

### AccessGlobals

This gates the ability to read and write static variables and globals.
Built-in functions that make use of mutable global state or expose the php-style superglobals will require this capability.

The `AccessGlobals` capability includes the `ReadGlobals` capability, so a function providing the `globals` context can call a function requiring the `read_globals` context.

```hack
class SomeClass {
  public static string $s = '';
}

function access_static()[globals]: void {
  SomeClass::$s; // OK
}
```

### System, SystemShallow, and SystemLocal

These capabilities are for Meta-internal uses.
<FbInternalOnly>See internal documentation here: https://www.internalfb.com/wiki/HackTutorials/LanguageBasics/Contexts_and_Capabilities/</FbInternalOnly>

### ImplicitPolicy, ImplicitPolicyShallow, ImplicitPolicyLocal, and ImplicitPolicyOf\<T>

These capabilities are for Meta-internal uses.
<FbInternalOnly>See internal documentation here: https://www.internalfb.com/wiki/HackTutorials/LanguageBasics/Contexts_and_Capabilities/</FbInternalOnly>

### Rx, RxShallow, and RxLocal

These capabilites are deprecated and unused.
<FbInternalOnly>See internal documentation here: https://www.internalfb.com/wiki/HackTutorials/LanguageBasics/Contexts_and_Capabilities/</FbInternalOnly>

## Contexts

### `defaults`

The `defaults` context has the capability set `{IO, WriteProperty, AccessGlobals, SystemLocal, ImplicitPolicyLocal, Codegen, RxLocal}`.

A function or method that doesn't specify a context list in its declaration has the `defaults` context. For example, this:

```hack
function f(): void {
  // ...
}
```

is equivalent to:

```hack
function f()[defaults]: void {
  // ...
}
```

Note that the capability set of `defaults` is not the universal set of capabilities.
A function with an unknown and unbounded (polymorphic) context cannot be called from an only `[defaults]` context.
There is no context from which any function can be called.

### The "Pure" Context

The empty context list, `[]`, has no capabilities. A function with no capabilities is the closest thing Hack has to 'pure' functions. As additional capabilities are added to Hack in the future, the restriction on these functions may increase.

As such, this is sometimes referred to as the 'pure context'.

A function or method with a pure context has limited capabilities:

- A function with this context can read object properties and constants (including class constants).
- A function with this context can only call functions or methods that also have a pure context.
- A function with this context cannot read or write static properties (it has neither `ReadGlobals` nor `AccessGlobals`).
- A function with this context cannot use `print` or `echo` (it does not have `IO`).
- A function with this context cannot change an object's properties (it does not have `WriteProperty`).
- Any function or method with a context can call a function or method with this context.

### `write_props`

The `write_props` context has the capability set `{WriteProperty}`.

### `read_globals`

The `read_globals` context has the `ReadGlobals` capability.

### `globals`

The `globals` context has the `AccessGlobals` capability.

### `zoned`, `zoned_shallow`, `zoned_local`, and `zoned_with<T>`

These capabilities are for Meta-internal uses.
<FbInternalOnly>See internal documentation here: https://www.internalfb.com/wiki/HackTutorials/LanguageBasics/Contexts_and_Capabilities/</FbInternalOnly>

### `leak_safe`, `leak_safe_shallow`, and `leak_safe_local`

These capabilities are for Meta-internal uses.
<FbInternalOnly>See internal documentation here: https://www.internalfb.com/wiki/HackTutorials/LanguageBasics/Contexts_and_Capabilities/</FbInternalOnly>

### `rx`, `rx_shallow`, and `rx_local`

These contexts are deprecated and unused.
<FbInternalOnly>See internal documentation here: https://www.internalfb.com/wiki/HackTutorials/LanguageBasics/Contexts_and_Capabilities/</FbInternalOnly>
