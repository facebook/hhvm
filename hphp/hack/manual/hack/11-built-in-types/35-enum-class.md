In comparison to [enumerated types (enums)](/hack/built-in-types/enum), enum classes are not restricted to int and string values.

## Enum types v. Enum class
Built-in enum types limit the base type of an enum to `arraykey` -- an integer or string -- or another enum.

The base type of an _enum class_ can be any type: they are not required to be constant expressions and objects are valid values. However, with Generics, if an enum has type `T` as a base type, its enum values are bound to values whose type are a subtype of `T`.

## Declaring a new enum class

Enum classes are syntactically different from [enum types](/hack/built-in-types/enum), as they require:

* the `enum class` keyword rather than the `enum` keyword
* that each value is annotated with its precise type: for example, `string` in `string s = ...`

### Example: Simple declarations

```Hack
// Enum class where we allow any type
enum class Random: mixed {
  int X = 42;
  string S = 'foo';
}

// Enum class that mimics a normal enum (only allowing ints)
enum class Ints: int {
  int A = 0;
  int B = 10;
}
```

### Example: Interface as a Base Type

```Hack file:hasname.hack
// Some class definitions to make a more involved example
interface IHasName {
  public function name(): string;
}

class HasName implements IHasName {
  public function __construct(private string $name)[] {}
  public function name(): string {
    return $this->name;
  }
}

class ConstName implements IHasName {
  public function name(): string {
    return "bar";
  }
}

// enum class which base type is the IHasName interface: each enum value
// can be any subtype of IHasName, here we see HasName and ConstName
enum class Names: IHasName {
  HasName Hello = new HasName('hello');
  HasName World = new HasName('world');
  ConstName Bar = new ConstName();
}
```

## Accessing values

Once declared, enum values are accessed using the `::` operator: `Names::Hello`, `Names::Bar`, ...

### Control over enum values

Using [coeffects](../contexts-and-capabilities/introduction.md), you can have control over which expressions are allowed as enum class constants.

By default, all enum classes are under the `write_props` context. It is not possible to override this explicitly.

## Extending an Existing enum class (Inheritance)

Enum classes can be composed together, as long as they implement the same base type:

```Hack file:extend.hack
interface IBox {}

class Box<T> implements IBox {
  public function __construct(public T $data)[] {}
}

enum class Boxes: IBox {
  Box<int> Age = new Box(42);
  Box<string> Color = new Box('red');
  Box<int> Year = new Box(2021);
}

function get<T>(\HH\MemberOf<Boxes, Box<T>> $box): T {
  return $box->data;
}

function test0(): void {
  get(Boxes::Age); // ok, of type int, returns 42
  get(Boxes::Color); // ok, of type string, returns 'red'
  get(Boxes::Year); // ok, of type int, returns 2021
}
```

```Hack file:extend.hack
enum class EBase: IBox {
  Box<int> Age = new Box(42);
}

enum class EExtend: IBox extends EBase {
  Box<string> Color = new Box('red');
}
```

In this example, `EExtend` inherits `Age` from `EBase`, which means that `EExtend::Age` is defined.

As with ordinary class extension, using the `extends` keyword will create a subtype relation between the enums: `EExtend <: EBase`.
Enum classes support multiple inheritance as long as there is no ambiguity in value names, and that each enum class uses the same base type:

```Hack file:extend.hack
enum class E: IBox {
  Box<int> Age = new Box(42);
}

enum class F: IBox {
  Box<string> Name = new Box('foo');
}

enum class X: IBox extends E, F { } // ok, no ambiguity


enum class E0: IBox extends E {
  Box<int> Color = new Box(0);
}

enum class E1: IBox extends E {
  Box<string> Color = new Box('red');
}

// enum class Y: IBox extends E0, E1 { }
// type error, Y::Color is declared twice, in E0 and in E1
// only he name is use for ambiguity
```

### Diamond shape scenarios
Enum classes support diamond shaped inheritance as long as there is no ambiguity, like in:

```Hack file:extend.hack
enum class DiamondBase: IBox {
  Box<int> Age = new Box(42);
}

enum class D1: IBox extends DiamondBase {
  Box<string> Name1 = new Box('foo');
}

enum class D2: IBox extends DiamondBase {
  Box<string> Name2 = new Box('bar');
}

enum class D3: IBox extends D1, D2 {}

<<__EntryPoint>>
function main(): void {
  echo D3::Age->data;
}
```

Here there is no ambiguity: the constant `Age` is inherited from `DiamondBase`, and only from there.
The `main` function will echo `42` as expected.

If either `D1`, `D2` or `D3` tries to define a constant named `Age`, there will be an error.

### Control over inheritance

Though the `final` keyword is not supported, Enum classes support the [`__Sealed`](../attributes/predefined-attributes#__sealed) attribute. Using `__Sealed`, you can specify which other enum classes, if any, are allowed to extend from your enum class.


## Abstract enum classes

Like regular classes, enum classes come in two flavors: concrete and abstract. An abstract enum class can declare abstract members (constants), where only their type and name are provided.

```Hack file:hasname.hack
// abstract enum class with some abstract members
abstract enum class AbstractNames: IHasName {
  abstract HasName Foo;
  HasName Bar = new HasName('bar');
}
```

Abstract members do not support default values, and can't be accessed directly. They only map a name to a type.
You must extend your abstract enum class into a concrete one with implementations of all abstract members to
safely access members defined as abstract.

```Hack file:hasname.hack
enum class ConcreteNames: IHasName extends AbstractNames {
  HasName Foo = new HasName('foo'); // one must provide all the abstract members
  // Bar is inherited from AbstractNames
}
```

All concrete members are inherited, and can't be overriden.



## Defining a Function that expects an enum class

When defining a function that expects an enum class value (e.g. `Foo::BAR`), you need to define the expected parameter appropriately with `HH\MemberOf` or you will run into errors.

```Hack error
enum class Foo: string {
  string BAR = 'BAZ';
}

function do_stuff(Foo $value): void {
  var_dump($value);
}

function main(): void {
  do_stuff(Foo::BAR); // expected Foo but got string
}
```

However, if we instead define `do_stuff()` as receiving `HH\MemberOf<Foo, string>`, then we can use `Foo::Bar` with no issues.

```Hack
enum class Foo: string {
  string BAR = 'BAZ';
}

function do_stuff(HH\MemberOf<Foo, string> $value): void {
  var_dump($value);
}

function main(): void {
  do_stuff(Foo::BAR); // ok
}
```

## Accessing enum class types

**An enum class type is more informative than a traditional built-in enum type.**

Let's examine `enum E` v. `enum class EC`.

```Hack
enum E: int {
  A = 42;
}
```

```Hack
enum class EC: int {
  int A = 42;
}
```

The built-in enum type of `E::A` is just `E`. All we know is that value `A` is declared within the enum: we know nothing of its underlying type.

But if we look at the enum class `EC::A` its type is `HH\MemberOf<EC, int>`. We know that it's declared within the enum class `EC`, with type `int`.

## Declaring type constants in an enum class

Like normal classes, enum classes can declare type constants. Abstract type
constants are also supported:

```Hack
interface IGet<+T> {
  public function get(): T;
}

class Box<T> implements IGet<T> {
  public function __construct(private T $data)[] {}
  public function get(): T { return $this->data; }
  public function set(T $data): void { $this->data = $data; }
}

abstract enum class E : IGet<mixed> {
  abstract const type T;
  abstract Box<this::T> A;
  Box<int> B = new Box(42);
}

enum class F : IGet<mixed> extends E {
  const type T = string;
  Box<this::T> A = new Box('zuck');
}
```

## Full Example: Dependent Dictionary

First, a couple of general Hack definitions:

```Hack file:dep_dict.hack
function expect_string(string $str): void {
  echo 'expect_string called with: '.$str."\n";
}

interface IKey {
  public function name(): string;
}

abstract class Key<T> implements IKey {
  public function __construct(private string $name)[] {}
  public function name(): string {
    return $this->name;
  }
  public abstract function coerceTo(mixed $data): T;
}

class IntKey extends Key<int> {
  public function coerceTo(mixed $data): int {
    return $data as int;
  }
}

class StringKey extends Key<string> {
  public function coerceTo(mixed $data): string {
    // random logic can be implemented here
    $s = $data as string;
    // let's make everything in caps
    return Str\capitalize($s);
  }
}
```

Now let’s create the base definitions for our dictionary

```Hack file:dep_dict.hack
enum class EKeys: IKey {
  // here are a default key, but this could be left empty
  Key<string> NAME = new StringKey('NAME');
}

abstract class DictBase {
  // type of the keys, left abstract for now
  abstract const type TKeys as EKeys;
  // actual data storage
  private dict<string, mixed> $raw_data = dict[];

  // generic code written once which enforces type safety
  public function get<T>(\HH\MemberOf<this::TKeys, Key<T>> $key): ?T {
    $name = $key->name();
    $raw_data = idx($this->raw_data, $name);
    // key might not be set
    if ($raw_data is nonnull) {
      $data = $key->coerceTo($raw_data);
      return $data;
    }
    return null;
  }

  public function set<T>(\HH\MemberOf<this::TKeys, Key<T>> $key, T $data): void {
    $name = $key->name();
    $this->raw_data[$name] = $data;
  }
}
```

Now one just need to provide a set of keys and extends `DictBase`:

```Hack file:dep_dict.hack
class Foo { /* user code in here */ }

class MyKeyType extends Key<Foo> {
  public function coerceTo(mixed $data): Foo {
    // user code validation
    return $data as Foo;
  }
}

enum class MyKeys: IKey extends EKeys {
  Key<int> AGE = new IntKey('AGE');
  MyKeyType BLI = new MyKeyType('BLI');
}

class MyDict extends DictBase {
  const type TKeys = MyKeys;
}
```

```Hack file:dep_dict.hack
<<__EntryPoint>>
function main(): void {
  $d = new MyDict();
  $d->set(MyKeys::NAME, 'tony');
  $d->set(MyKeys::BLI, new Foo());
  // $d->set(MyKeys::AGE, new Foo()); // type error
  expect_string($d->get(MyKeys::NAME) as nonnull);
}
```
