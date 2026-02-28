# Implementing an Interface

A class can implement a *contract* through an interface, which is a set of required
method declarations and constants.

Note that the methods are only declared, not defined; that is, an interface defines a type consisting
of *abstract* methods, where those methods are implemented by client classes as they see fit. An interface allows unrelated classes to
implement the same facilities with the same names and types without requiring those classes to share a common base class. For example:

```hack
interface MyCollection {
  const MAX_NUMBER_ITEMS = 1000;
  public function put(int $item): void;
  public function get(): int;
}

class MyList implements MyCollection {
  public function put(int $item): void { /* implement method */ }
  public function get(): int { /* implement method */
    return 0;
  }
  // ...
}

class MyQueue implements MyCollection {
  public function put(int $item): void { /* implement method */ }
  public function get(): int { /* implement method */
    return 0;
  }
  // ...
}

function process_collection(MyCollection $p1): void {
  /* can process any object whose class implements MyCollection */
  $p1->put(123);
}

<<__EntryPoint>>
function main(): void {
  process_collection(new MyList());
  process_collection(new MyQueue());
}
```

In this example, we define an interface type called `MyCollection` that contains an implicitly static [constant](/hack/classes/constants) and two implicitly
abstract methods.  Note how these methods have no bodies; their declarations end in a semicolon, which makes them abstract.  Next, we define two
classes that each implement this interface.

Note carefully that the parameter type of `process_collection` is an interface type. As such, when that function is called, the argument can
have any type that implements that interface type.  As we add new collection types that implement that interface, we can plug them into the
application without impacting existing code.

An interface can extend another interface; for example:

```hack no-extract
interface Iterator<Tv> extends Traversable<Tv> {
  // ...
}
```

The library interface generic type `Iterator<...>` inherits the members of the interface generic type `Traversable<...>`.  The `extends`
clause allows a comma-separated list of base interfaces, so an interface can have multiple base interfaces.  As such, the members of an
interface are those specified by its own declaration, and the members inherited from its base interfaces.

Interfaces are designed to support classes; an interface cannot be instantiated directly.

An interface can have usage requirements placed on it; see [interface requirements](/hack/traits-and-interfaces/trait-and-interface-requirements) for more information.

Methods declared in interfaces, and abstract methods in classes, should not use default values for parameters. Instead, they can declare a
parameter to be `optional`. Any implementation of the interface or abstract method must supply default values for the optional parameters.
```hack
interface ICounter {
  public function inc(optional int $increment): void;
  public function get(): int;
}
class Counter implements ICounter {
  public function __construct(private int $value = 0) { }
  public function inc(int $increment = 1): void {
    $this->value += $increment;
  }
  public function get(): int {
    return $this->value;
  }
}
function doSomething(ICounter $c):void {
  $c->inc(); // Use default as defined by implementation of ICounter
  $c->inc(10); // Uses the supplied value
}
```
