An expression to get the name of a class as a string, replacement for
[`::class` expressions](/hack/expressions-and-operators/scope-resolution) in
string positions.

## Quickstart

```Hack
class C {}
function expect_string(string $s): void {}
function test(): void {
    expect_string(nameof C);
    $d = dict[];
    $d[nameof C] = 4;
}
```

## Full Example

`nameof` supports direct class names, traits and interfaces, enums, and type aliases.
It also supports the special class identifiers `self`, `static`, and `parent` where
they are valid.

```Hack
class B {}
class C extends B {}
trait T {
    require extends B;
    public static function f(): void {
        var_dump(nameof T); // "T"
        var_dump(nameof self); // "D" (user of trait)
        var_dump(nameof parent); // "C" (parent of trait user, not B)
        var_dump(nameof static); // "E", receiver for ::f() in main
    }
}
class D extends C { use T; }
class E extends D {}

<<__EntryPoint>>
function main(): void {
  E::f();
}
```

The type of a `nameof Target` expression is
[`classname<Target>`](/hack/built-in-types/classname) and `typename<Target>` when `Target`
is a type alias. [Reified generics](/hack/reified-generics/reified-generics) are not 
supported targets for `nameof`.
