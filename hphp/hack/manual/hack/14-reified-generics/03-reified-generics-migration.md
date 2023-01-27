# Migration Features for Reified Generics

In order to make migrating to reified generics easier, we have added the following migration features:

* The `<<__Soft>>` annotation on a type parameter implies the intent to switch it to reified state. If generics at call sites are not provided the type checker will raise an error, and the runtime will raise a warning so you can capture these locations through logging. The generic cannot be used in the body of the function.
* The `<<__Warn>>` annotation on a type parameter implies that if the reified generic is incorrect at parameter or return type hints locations, the runtime will raise a warning instead of type hint violation.
* The `get_type_structure<reify T>()` function, given a reified type, returns the type structure representing the type.
* The `get_classname<reify T>()` function, given a reified type, returns the name of the class represented by this type, or raises an exception if the type does not represent a class.

## Example Incremental Migration

In this part, we'll walk you through how to migrate a non reified function to a reified function. Some of these steps can be skipped depending on the use case.

0) Beginning of time

```Hack
class C<T> {}
function f(C<int> $x): void {}

function demo(): void {
  f(new C()); // OK
}
```

1) You have managed to write out all the type annotations (either pre-existing or by using `<<__Soft>>` annotation logging)

```Hack error
class C<<<__Soft>> reify T> {}
function f(C<int> $x): void {}

function demo(): void {
  f(new C<string>()); // Typechecker error: string incompatible with int
}
```

2) You now want to remove `__Soft` and start using the generic. So you move `__Soft` to `__Warn`.

```Hack error
class C<<<__Warn>> reify T> {}
function f(C<int> $x): void {}

function demo(): void {
  f(new C<string>()); // Runtime warning: string incompatible with int
}
```

3) By using logging, you have added `__Soft` to everywhere it's used and now it will be safe to remove `__Warn`.

```Hack error
class C<<<__Warn>> reify T> {}
function f(C< <<__Soft>> int> $x): void {}

function demo(): void {
  f(new C<string>()); // Runtime warning: string incompatible with int
}
```

4) `__Warn` goes away

```Hack error
class C<reify T> {}
function f(C< <<__Soft>> int> $x): void {}

function demo(): void {
  f(new C<string>()); // Runtime warning: string incompatible with int
}
```

5) Fix the use site

```Hack
class C<reify T> {}
function f(C<string> $x): void {}

function demo(): void {
  f(new C<string>()); // OK
}
```
