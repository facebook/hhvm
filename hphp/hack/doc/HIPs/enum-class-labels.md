# HIP: Enum Class Labels

### Status: Candidate

**Depends on:**  Enum Classes project.

**Implementation status:** Done. Depends on a couple of lags

* ~~Enum classes flag, now enabled by default. (HHVM: `-v Hack.Lang.EnableEnumClasses=1`)~~ This is now removed
* ~~Unstable Features `enum_class_label` (HHVM: `-v Hack.Lang.AllowUnstableFeatures=1`)~~ No longer an unstable feature
* ~~common `disallow-hash-comments` flag~~ this flag has been removed from the parser.

## Motivation

Enums and Enum classes are sets of values. Enum constants and enum class values can be accessed via bindings, but the bindings themselves do not have denotable expressions.  Consider for instance:

```
enum E : int {
   A = 42;
   B = 42;
}

function f(E $x): void {
  switch ($x) {
    case E::A: echo "from A I get "; break;
    case E::B: echo "from B I get "; break;
  }
  echo "$x\n";
}

function test(): void {
  // Both echoes "from A I get 42"
  f(E::A);
  f(E::B);
}
```

In the code above, `E::A` and  `E::B` are values of type `E` and the programmer used the `switch` statement to recover which binding was used to access the enum constant bound to `$x`.  However, since the runtime is only aware of enum constants and not bindings, in both invocations of `f` the runtime will always match the constant `42` (bound in both cases to `$x`) against the constant `42` (obtained via `E::A` in the first case statement): in both invocations the first case statement is selected and `"from A I get 42"` is displayed twice.

While using Enum classes to replace codegen with generic programming, we faced some situations where we needed these bindings to be denotable and first class.  Neither enums, nor enum classes, provide this at the moment.  We also realised that using enum classes in www increases verbosity compared to the old codegened APIs: a call to an “old-fashioned" codegened `getName()` method might now look like `get(MyLongClassNameParameters::Name)`.


## Proposal

This proposal introduces the concept of **labels** for **enum class** constants.  Fully qualified labels are new expressions of the form `Foo#Bar`, where `Foo` is the name of an enum class and `Bar` is the one of `Foo`’s constant. Labels provide first-class bindings for enum classes and can be used in two new builtin functions to obtain the name and value of the corresponding enum class constant. Labels also have a short syntax, of the form `#Bar`, that limits verbosity whenever type inference can be leveraged to reconstruct internally the full `Foo#Bar` label.

**Expressions & types.** A label from the enum class `Foo` binding a constant to the type `T` has type `HH\EnumClass\Label<Foo, T>`.  The new type `HH\EnumClass\Label` is opaque.

```
enum class E : I {
  C Name = ...;
}

E#Name : HH\EnumClass\Label<E, C>

$x = E#Name; // ok
f(E#Name);   // ok
```

In practice, Hack often has enough information to infer the correct enum class name, so we can make it implicit: the `f(E#Name)` call can then be shortened as `f(#Name)`. When type information are not sufficient, Hack reports an error and asks for a fully qualified annotation. We currently only allow the short form in some function calls, but we plan to allow more locations in the future.

```
$x = #Name; // error, not enough context to infer the enum
f(#Name);   // ok, because Hack knows f's first arg type. It behaves like f(E#Name)
```


**New builtin functions.** This proposal extends the definition of enum classes to provide two new static methods that can consume labels:

* `nameOf` which takes a label and returns a string representation of it
* `valueOf` which takes a label and returns the value associated with this label

Here is a short example of their use:

```
function show_label<T>(HH\EnumClass\Label<E, T> $label) : void {
  echo E::nameOf($label);
}

function get<T>(HH\EnumClass\Label<E, T> $label) : T {
  return E::valueOf($label);
}
```

In this example:

* `show_label(E#Name)` will display a string representation of `E#Name`. For now, it would display the string `Name`.
* `get(E#Name)` returns the value `E::Name`.


As long as Hack can infer the enum class name, fully qualified and short expressions can be used transparently:
Since `get(#Name)` behaves like `get(E#Name)`, it will return the same value `E::Name`.

### Sugar coating requested by Beta testers

As requested by some beta testers, we also provide an alternative way write the special case where the **first** function argument is a label, allowing to write `f(#X, ...)` as `f#X(...).` This will enable the developers to use a syntax closer to the old-fashioned  `getName()`, by writing `get#Name()`.

Note that this is only syntactic sugar and does not provide additional possibilities like taking a function pointer or a lambda: `get#Name<>` is forbidden, as it would be a function pointer on a partially applied function.

### IDE Integration

* Automatic completion provides constant names when writing a valid prefix like `getParam#` (implemented by Vassil)
* Documentation hover shows as hover types with doc for labels and function calls sugar like `get#Name()` (implemented by Tom)

## Implementation

### Forewords

It is important to note that until recently, the `#` symbol was a marker for a one line comment.  This has been removed, and the  `#` symbol is now reused by this proposal.  This is not a limitation as Hack still supports one line comments using the `//` syntax.

### Runtime

This proposal introduces two new expressions in the AST, the fully qualified enum class labels (`E#Name`) and the short form (`#Name`).   The current sample runtime implementation works as follow:

* both fully qualified labels and  short labels are compiled to the `Name` label identifier, discarding the `E` part;
*  the base class of enum classes, `BuiltinEnumClass`, is extended with two new final static methods, `nameOf` and `valueOf`.  Given a label identifier, and the class name they are invoked on, these can perform name and value resolution via a special HHVM intrinsics.

### Type checking

To type fully qualified enum class labels, we introduce a new type `HH\EnumClass\Label<-Tenum, Tdata>` which reflects the enum class where the label is defined, and its associated type. The variance of the first type parameter is the same as the one for the `HH\MemberOf` type, introduced by enum classes.  Please refer to the enum class HIP for a detailed discussion of  variance.

The use of fully qualified enum class labels does not require any type checker change.  The shorter version instead needs minor changes to type checking function calls.  We detail how Hack’s type inference is modified to reconstruct the enum class name.

Let us do one step backward and consider Hack's handling of function calls.  At any point in a program, Hack has access to the signature of all previously defined functions.  Suppose that Hack is inferring the type of the expression `f(expr)`:

* it already knows the type of the top level function/method `f` from the context : `A -> B`
* it recursively infers the type of `expr` : `C`
* it checks that `C` is a valid subtype of `A` : `C <: A`
* if that’s the case, the whole expression has type `B`

In general the type of `f` is not needed to infer the type of `expr`.   However, when short syntax is encountered, the type information about `f` arguments is used to infer the type of a label. As an example, let us consider again the `get` function:

```
function get<T>(HH\EnumClass\Label<E, T> $x) : T
```

When inferring the type of `get(#Name)` Hack can perform the following steps:

* from the context, it knows that `get` has type `HH\EnumClass\Label<E, T> -> T`
* it recursively infers the type of `#Name`, *knowing that a constant from `E` is expected*
* there is indeed a constant named `Name` in the enum class `E` so the call is valid
* the full expression will thus behave as if the developer wrote `get(E#Name)`

We currently support this mechanic only for function call when the top level type is a `Label`. In the next section, we’ll discuss ways to support the short syntax in more locations, but this HIP only guarantees the support for function calls.

## Future extensions

### Extension to enumeration types

Standard enumeration types could benefit from the same usage of fully qualified and short labels.  In this proposal, we only focus on enum classes, but the similar machinery might be ported to enums, and extend the expressivity of labels to them. Since there is currently no demand for it, we restrict this feature to enum classes for now.

The main differences  we expect are:

* A dedicated label type, `HH\Enum\Label<Tenum, Tdata>` with different constraints on the type parameter (`Tenum` will probably be invariant since enum inclusion does not entail subtyping, and `Tdata` constraint by `arraykey`)
* The `nameOf` and `valueOf` methods would have to be defined in the `BuiltinEnum` class, and the signature of `valueOf` would need to be adapted.

### Allowing short syntax at more locations

Internally Hack keeps an optional **expected type** information when type checking expressions. We plan to use this information to allow the short labels more broadly, e.g. in return statements.

### Performance sensitive code

The current proposal only addresses the verbosity issue for labels.  For enum class constant themselves, the full name must still be passed to functions that expect constants, not labels, as in the example below:

```
function get<T>(HH\MemberOf<E, T> $member) : T {
  return $member;
}

get(E::Name); // ok
get(#Name);   // not allowed
```

A solution to this issue is to call `valueOf` by hand and rewrite this `get` method like this:

```
function get<T>(HH\EnumClass\Label<E, T> $label) : T {
  return E::valueOf($label);
}

get(E::Name); // not allowed
get(#Name);   // ok
```

In performance-sensitive code, this extra function call might be too expensive.  **_If such code exists_** and the shorter enum class invocation syntax needs to be supported, we might provide an attribute that enables the short notation for constants too, eg.:

```
function get_via_label<T>(<<__ViaLabel>>HH\MemberOf<E, T> $member) : T {
  return $member;
}

get(E::Name); // not allowed
get(#Name);   // ok
```

The logic added to deal with labels has been added at function definition sites:  until more type information is available at compile time, or information from HHBBC can be exploited, we cannot support `<<__ViaLabel>>` by instrumenting call sites. Paul Bissonnette has ideas for that, but it won’t be discussed in this proposal.

We are emitting a special byte code sequence when `__ViaLabel` is used to transform the label (just a constant name) into the right class constant access, in the function **prologue**. This implies that inside `get_via_label`, even if called with the label `#Name`, `$member` is the *constant* value `E::Name`, not the label. There is no way to fetch back the label.


We currently have a running prototype to test this feature, but I don’t think we should ship this unless there are effectively some performance issues in the framework that will use labels.
