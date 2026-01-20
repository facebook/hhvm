# Inheritance

Hack supports single inheritance between classes.

``` Hack
class IntBox {
  public function __construct(protected int $value) {}

  public function get(): int {
    return $this->value;
  }
}

class MutableIntBox extends IntBox {
  public function set(int $value): void {
    $this->value = $value;
  }
}
```

Classes can access things defined in the parent class, unless they are
`private`.

## Overriding Methods

You can override methods in subclasses by defining a method with
the same name.

``` Hack
class IntBox {
  public function __construct(protected int $value) {}

  public function get(): int {
    return $this->value;
  }
}

class IncrementedIntBox extends IntBox {
  <<__Override>>
  public function get(): int {
    return $this->value + 1;
  }
}
```

If a method is intended to override a method in a parent class, you
should annotate it with `<<__Override>>`. This has no runtime effect, but
ensures you get a type error if the parent method is removed.

Hack does not support method overloading. Subclasses methods must have
a return type, parameters and visibility that is compatible with the
parent class.

``` Hack
class NumBox {
  public function __construct(protected num $value) {}

  protected function get(): num {
    return $this->value;
  }
}

class FloatBox extends NumBox {
  <<__Override>>
  public function get(): float {
    return (float)$this->value;
  }
}
```

The only exception is constructors, which may have incompatible
signatures with the parent class. You can use `<<__ConsistentConstruct>>`
to require subclasses to have compatible types.

``` Hack
class User {
  // This constructor takes one argument.
  public function __construct(protected string $name) {}
}

class Player extends User {
  // But this constructor takes two arguments.
  <<__Override>>
  public function __construct(protected int $score, string $name) {
    parent::__construct($name);
  }
}
```

## Calling Overridden Methods

You can use `parent::` to call an overridden method in the parent
class.

``` Hack
class IntBox {
  public function __construct(protected int $value) {}

  public function get(): int {
    return $this->value;
  }
}

class IncrementedIntBox extends IntBox {
  <<__Override>>
  public function get(): int {
    return parent::get() + 1;
  }
}
```

This also works for static methods.

``` Hack
class MyParent {
  public static function foo(): int {
    return 0;
  }
}

class MyChild extends MyParent {
  <<__Override>>
  public static function foo(): int {
    return parent::foo() + 1;
  }
}
```

## Abstract Classes

An abstract class cannot be instantiated. Attempting to create an instance of an abstract class or call an unimplemented abstract method causes a runtime error. The type checker helps prevent these errors. Given the following code:

``` Hack
abstract class Animal {
  public abstract function greet(): string;
}

class Dog extends Animal {
  <<__Override>>
  public function greet(): string {
    return "woof!";
  }
}
```

`new Dog()` is allowed but not `new Animal()`.

Abstract classes are similar to
[interfaces](/hack/traits-and-interfaces/implementing-an-interface), but they can include
implementations of methods.

### Static Methods and Late Static Binding

Abstract classes can have abstract static methods. When you use `static::` inside a method, [late static binding](/hack/expressions-and-operators/scope-resolution) determines which class's method runs based on the runtime class.

This creates a potential problem: what if someone calls your method in a context where `static` refers to an abstract class?

```hack warning
abstract class Animal {
  public static function introduce(): void {
    echo "I say: ";
    static::speak(); // Warning: static might be abstract at runtime
  }

  public static abstract function speak(): void;
}

class Dog extends Animal {
  public static function speak(): void {
    echo "Woof!";
  }
}
```

When you call `Dog::introduce()`, `static::speak()` successfully resolves to `Dog::speak()`. But if someone calls `Animal::introduce()` directly, `static::speak()` calls an abstract method, causing a runtime error.

The type checker tries to prevent such problems. To fix the warning, use the [`<<__NeedsConcrete>>`](/hack/attributes/predefined-attributes#__needsconcrete) attribute:

```hack warning
abstract class Animal {
  <<__NeedsConcrete>>
  public static function introduce(): void {
    echo "I say: ";
    static::speak(); // OK: attribute indicates `static` should resolve to a concrete class
  }

  public static abstract function speak(): void;
}
```

This tells the type checker that `introduce()` should only be called when the runtime class is concrete. See the [\_\_NeedsConcrete documentation](/hack/attributes/predefined-attributes#__needsconcrete) for more details.

## Final Classes

A final class cannot have subclasses.

``` Hack
final class Dog {
  public function greet(): string {
    return "woof!";
  }
}
```

If your class has subclasses, but you want to prevent additional
subclasses, use `<<__Sealed>>`.

If you want to inherit from a final class for testing, use
`<<__MockClass>>`.

You can also combine `final` and `abstract` on classes. This produces
a class that cannot be instantiated or have subclasses. The class is
effectively a namespace of grouped functionality.

``` Hack
abstract final class Example {
  public static function callMe(int $i): int {
    return static::helper($i);
  }

  private static function helper(int $i): int {
    return $i + 1;
  }
}
```

## Final Methods

A `final` method cannot be overridden in subclasses.

``` Hack
class IntBox {
  public function __construct(protected int $value) {}

  final public function get(): int {
    return $this->value;
  }
}
```
