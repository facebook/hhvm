# HIP: `nameof`

## Summary

A new way to get class names as strings: `nameof C`.

> NOTE: C is a stand-in for an `Aast.class_id`, excluding `Aast.CIexpr`.

## Feature motivation

This feature is meant to replace the use of `C::class` expressions in
string positions that trigger an implicit cast, so the implicit behavior can be
eliminated. Typing mimics `C::class` to maintain types during migration,
while compilation mimics `(string)C::class` to model existing implicit string coercion.

The feature also supports the migration of places where package boundaries
are violated, so that `C::class` can be changed to return a validated class
pointer. We will then ban and eliminate implicit and explicit `(string)`
conversion in the runtime.

## Specification

The expressions valid on the left hand side of `C::class` are
1. Classes
2. Interfaces
3. Traits
4. Enums / enum classes
5. `static`, `self`, and `parent`
6. Reified generics in scope
7. Type aliases

`nameof` supports 1-5 as targets. The confusion around 6 was significant enough
to where we've decided to exclude it, and 7 can wait until the semantics of what
`MyTypeAlias::class` should mean are nailed down. Other top-level identifiers
are similarly unsupported by nameof at the moment, such as `nameof my_function`,
`nameof MY_CONST` and `nameof my.module`, and Hack will error as though they
are missing (or illegal in the case of `my.module`) class names.

Let's proceed by example. Notation is `id: type = bytecode`.

### Class
```
class B extends A {
  public function f(): void {
    $b = nameof B; // $b: classname<B> = String "B"
    $s = nameof self; // $s: classname<B> = String "B"
    $p = nameof parent; // $b: classname<A> = String "A"
    $stat = nameof static; // $stat: classname<this> = LateBoundCls; ClassName
  }
}
```

### Parent undefined
```
class B {
  public function f(): void {
    $b = nameof parent; // $b: classname<nothing> (Typing[4076]) = ParentCls; ClassName
  }
}
```

### Interface
```
interface I extends I2 {
  const string S = nameof I; // S = """l:1:\"I\";""";
  const string T = nameof self; // T = """l:1:\"I\";""";

  // matches parent::class and static::class behavior
  const string U = nameof parent; // Parsing[1002] invalid constexpr
  const string V = nameof static; // Parsing[1002] invalid constexpr
}
```

### Interface require extends (same as previous)
```
interface I {
  require extends I2;

  const string S = nameof I; // S = """l:1:\"I\";""";
  const string T = nameof self; // T = """l:1:\"I\";""";
  const string U = nameof parent; // Parsing[1002] invalid constexpr
  const string V = nameof static; // Parsing[1002] invalid constexpr
}
```

### Trait
```
class A {}
trait T {
  require extends A;

  public function f(): void {
    // Types are sketchy but match T::class and self::class for migration
    $t = nameof T; // $t: classname<T> = String "T"
    $s = nameof self; // $s: classname<T> = SelfCls; ClassName

    $p = nameof parent; // $p: classname<A> = ParentCls; ClassName
    $stat = nameof static; // $stat: classname<this> = LateBoundCls; ClassName
  }
}
```

### Trait no parent
```
trait T {
  public function f(): void {
    $p = nameof parent; // $p: classname<nothing> (Typing[4074]) = ParentCls; ClassName
  }
}
```

## User Experience

`nameof` is a new keyword in VSCode's grammar, highlighted in blue with default
settings. Class names are autocompleted as they would be for `new`, without the
restriction on concrete classes.

## Unsupported

### Type aliases
```
class C {}
type X = C; // does not matter that X points to class
function f(): void {
  $x = nameof X; // $x: classname<nothing> (Naming[2052]) = "X"
}
```

### Reified generics
```
class C {}
function f<reify T as C>(): void {
  $t = nameof T; // $x: classname<T> (Parsing[1002]) = <bytecode_arg>
}
class R<reify Tr as C> {
  public function m<reify Tm as C>(): void {
    $t = nameof Tr; // $x: classname<Tr> (Parsing[1002]) = <bytecode_prop>
    $t = nameof Tm; // $x: classname<Tm> (Parsing[1002]) = <bytecode_arg>
  }
}

/**
 * This specifies the current behavior of T::class when T is a reified generic,
 * which is matched by the nameof implementation. The main difference is that
 * function-bound reified generics are parameters and class-bound ones are
 * properties. Since they're type structures, we just query the 'classname'
 * field and throw if it doesn't exist e.g. type structure for `int`. The
 * typechecker raises an error if the generic is not bounded by a class type.
 *
 * <bytecode_arg>
 *
 * BaseL $0ReifiedGenerics Warn Any
 * QueryM 0 CGet EI:0 Any
 * BaseC 0 Warn
 * QueryM 1 CGet ET:"classname" Any
 *
 * <bytecode_prop>
 *
 * CheckThis
 * BaseH
 * Dim Warn PT:"86reified_prop" Any
 * QueryM 0 CGet EI:0 Any
 * BaseC 0 Warn
 * QueryM 1 CGet ET:"classname" Any
 */
```
