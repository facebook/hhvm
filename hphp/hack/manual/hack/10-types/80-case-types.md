# Case Types

Case types are a special kind of type alias that enable declaration of runtime-disjoint unions.

## Basic Syntax

```
case type Name = Variant1 | Variant2 | ... ;
case type Name<T1, T2> as UpperBound = Variant1 | Variant2 | ... ;
```

**Key Components:**
- **Name** The identifier for the case type
- **Type parameters** (optional)
- **Upper bound** (optional) Declared with `as`, implicitly `mixed` if not specified
- **Variants** The component type(s), separated by `|`

### Examples

```hack
class MyClass {}

case type SimpleCaseType = int | MyClass;

case type BoundedAndGenericCaseType<+Tk as arraykey> as nonnull =
  keyset<Tk> | int;
```

## The Runtime-Disjoint Requirement

The variants of a case type must be known to be shallowly disjoint at runtime.
This is defined by mapping each type to a set of runtime data tags.
Two types are runtime-disjoint if they do not share any runtime tags.

The purpose of this requirement is to enable runtime-efficient, exhaustive decomposition of the type that fully recovers type information.

**Example of runtime data tags:**
- Primitive tags: `int`, `bool`, `float`, `string`, `null`
- Array tags: `vec`, `dict`, `keyset`
  - Note that shapes and tuples are dicts and vecs respectively at runtime and so do not have their own distinct tag.
  - Note that array types' generics are erased and so do not affect the tag.
- Object tags: There is a tag for each class.
  - Note that only reified generics are included in the tag.
  - Note that disjointness accounts for inheritance. For example, two unsealed interface types are not considered disjoint because a single class could implement both. But two abstract classes are disjoint.

### Valid (Disjoint) Examples

```hack
// Different primitives
case type Good1 = int | string | bool;

// Different array kinds
case type Good2 = vec<int> | dict<int, int>;

// vec vs shape
case type Good3 = vec<int> | shape('x' => int);

// Final class vs interface - it is known that C does not implement I2
final class C {}
interface I2<T> {}
case type Good4 = C | I2<int>;

// due to the `as arraykey` bound, we know that `T` is disjoint from `float`
case type GenericCaseType<T as arraykey> = T | float;
```

### Invalid (Overlapping) Examples

```hack
// vec and Traversable overlap (vec implements Traversable)
case type Bad1 = vec<int> | Traversable<string>;

// Two interfaces can overlap (a class could implement both)
interface I1 {}
interface I2 {}
case type Bad2 = I1 | I2;

// vec and tuple have overlapping runtime representation
case type Bad3 = vec<int> | (int, int);

// shape fields are not considered
case type Bad4 = shape('x' => int) | shape('y' => string);

// E is a subtype of C, so they overlap
class C {}
class E extends C {}
case type Bad5 = C | E;
```

## Subtyping

The treatment of case types differs when it appears on different sides of a subtyping check.
i.e. "Is a value of type CaseType a value of type T?" (`CaseType <: T`) v.s. "Is a value of T a subtype of a value of type CaseType?" (`T <: CaseType`).

The purpose of this asymmetry is to avoid quadratic performance costs related to subtyping union types.

### When Case Type is on the Super Side (Right Side)

When a case type appears on the **right side** of a subtype check (as a supertype), it is **expanded to its union**:

```hack
case type CT = int | string;

// When CT is expected (super side), we can pass int or string
function accept_ct(CT $x): void {}

function f(): void {
  accept_ct(42);      // OK: int <: (int | string)
  accept_ct("hello"); // OK: string <: (int | string)
}
```

### When Case Type is on the Sub Side (Left Side)

When a case type appears on the **left side** of a subtype check (as a subtype), its **upper bound is used** instead of the union:

```hack
case type CT_Bounded as arraykey = int;
case type CT_No_Bounds = int;

function expect_int(int $x): void {}
function expect_arraykey(arraykey $x): void {}
function expect_mixed(mixed $x): void {}

function test(CT_Bounded $bounded, CT_No_Bounds $unbounded): void {
  expect_int($bounded);       // ERROR: arraykey </: int
  expect_arraykey($bounded);  // OK: arraykey <: arraykey
  expect_mixed($bounded);     // OK: arraykey <: mixed

  expect_int($unbounded);     // ERROR: mixed </: int
  expect_arraykey($unbounded);// ERROR: mixed </: arraykey
  expect_mixed($unbounded);   // OK: mixed <: mixed
}
```

## Decomposition

Case types can be decomposed using `is` runtime type checks:

```hack
final class MyClass {}

case type MyCaseType = int | string | MyClass;

function takes_case_type(MyCaseType $x): void {
  if ($x is int) {
    // $x : int
  } else {
    // $x : string | MyClass
    if ($x is string) {
      // $x : string
    } else {
      // $x : MyClass
    }
  }
}
```
