# Interfaces

## General

A class can implement a set of capabilities—herein called a
*contract*—through what is called an interface. An *interface* is a set
of method declarations and constants.  Note that the methods are only
declared, not defined; that is, an interface defines a type consisting
of abstract methods, where those methods are implemented by client
classes as they see fit. An interface allows unrelated classes to
implement the same facilities with the same names and types without
requiring those classes to share a common base class.

An interface can extend one or more other interfaces, in which case, it
inherits all members from its *base interface(s)*.

## Interface Declarations

**Syntax**

<pre>
  <i>interface-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  interface  <i>name</i>  <i>generic-type-parameter-list<sub>opt</sub></i>  <i>interface-base-clause<sub>opt</sub></i> {
      <i>interface-member-declarations<sub>opt</sub></i>  }

  <i>interface-base-clause:</i>
    extends  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
    <i>interface-base-clause</i>  ,  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
</pre>

**Defined elsewhere**

* [*attribute-specification*](21-attributes.md#attribute-specification)
* [*generic-type-parameter-list*](14-generic-types-methods-and-functions.md#type-parameters)
* [*generic-type-argument-list*](14-generic-types-methods-and-functions.md#type-parameters)
* [*interface-member-declarations*](17-interfaces.md#interface-members)
* [*name*](09-lexical-structure.md#names)
* [*qualified-name*](20-namespaces.md#defining-namespaces)

**Constraints**

An interface must not be derived directly or indirectly from itself.

*qualified-name* must name an interface type.

A generic interface and a non-generic interface in the same scope cannot have the same *name*.

**Semantics**

An interface-declaration defines a contract that one or more classes can
implement.

Interface names are case-insensitive.

The optional *interface-base-clause* specifies the base interfaces from
which the interface being defined is derived. In such a case, the
derived interface inherits all the members from the base interfaces.

**Examples**

```Hack
interface MyCollection<T> {
  const MAX_NUMBER_ITEMS = 1000;
  public function put(T $item): void;
  public function get(): T;
}
class MyList<T> implements MyCollection<T> {
  public function put(T $item): void    { /* implement method */ }
  public function get(): T          { /* implement method */ }
  …
}
class MyQueue<T> implements MyCollection<T> {
  public function put(T $item): void    { /* implement method */ }
  public function get(): T          { /* implement method */ }
  …
}
function processCollection<T>(MyCollection<T> $p1): void {
  … /* can process any object whose class implements MyCollection
}
processCollection(new MyList(…));
processCollection(new MyQueue(…));
```

## Interface Members

**Syntax**

<pre>
  <i>interface-member-declarations:</i>
    <i>interface-member-declaration</i>
    <i>interface-member-declarations   interface-member-declaration</i>
  <i>interface-member-declaration:</i>
    <i>require-extends-clause</i>
    <i>const-declaration</i>
    <i>method-declaration</i>
    <i>type-constant-declaration</i>
</pre>

**Defined elsewhere**

* [*const-declaration*](16-classes.md#constants)
* [*method-declaration*](16-classes.md#methods)
* [*require-extends-clause*](18-traits.md#trait-members)
* [*type-constant-declaration*](16-classes.md#type-constants)

**Constraints**

The *qualified-name* in *requires-extends-clause* must designate a class name.

**Semantics**

The members of an interface are those specified by its
*interface-member-declaration*s, and the members inherited from its base
interfaces.

An interface may contain the following members:

* *require-extends-clauses* each of which requires the class implementing this interface to directly or indirectly extend the class type designated by *qualified-name*.
* [Constants](17-interfaces.md#constants) – the constant values associated with the interface.
* [Methods](17-interfaces.md#methods) – placeholders for the computations and actions that can be performed by implementers of the interface.
*	[Type constants](16-classes.md#type-constants) – a way of parameterizing class types without using generics.

An *interface-member-declarations* may contain multiple *require-extends-clause*s that designate the same class, in which case, the duplicates are redundant. 

## Constants

**Semantics**

An interface constant is just like a [class constant](16-classes.md#constants), except that
an interface constant cannot be overridden by a class that implements it
nor by an interface that extends it.

**Examples:**

```Hack
interface MyCollection<T> {
  const MAX_NUMBER_ITEMS = 1000;
  public function put(T $item): void;
  public function get(): T;
}
```

## Methods

**Constraints**

Methods declared in an interface must not be declared `abstract`.

An interface method must not also be asynchronous. However, a method can have a return-type of `Awaitable<T>`, so an async concrete implementation can be provided.

**Semantics:**

An interface method is just like an [abstract method](16-classes.md#methods).

**Examples:**

```Hack
interface MyCollection<T> {
  const MAX_NUMBER_ITEMS = 1000;
  public function put(T $item): void;
  public function get(): T;
}
```

## Predefined Interfaces

### Interface `ArrayAccess`

This interface allows an instance of an implementing class to be
accessed using array-like notation. This interface is defined, as
follows:

```Hack
interface ArrayAccess<string, T> {
  public function offsetExists(string $offset): bool;
  public function offsetGet(string $offset): T;
  public function offsetSet(string $offset, T $value): this;
  public function offsetUnset(string $offset): this;
```

The interface members are defined below:

Name	|   Purpose
----    |   -------
`offsetExists`  |	This instance method returns `true` if the instance contains an element with key `$offset`, otherwise, `false`.
`offsetGet`	|  This instance method gets the value having key `$offset`. This method is called when an instance of a class that implements this interface is [subscripted](10-expressions.md#subscript-operator) in a non-lvalue context.
`offsetSet`	| This instance method sets the value having key `$offset` to $value. This method is called when an instance of a class that implements this interface is [subscripted](10-expressions.md#subscript-operator) in a modifiable-lvalue context.
`offsetUnset`	| This instance method removes the value having key `$offset`.

### Interface `AsyncIterator`

This interface supports iteration over the values returned from an [asynchronous generator function](10-expressions.md#yield-operator). It is defined, as follows:

```Hack
interface AsyncIterator<Tv> {
  public function next(): Awaitable<?tuple<mixed,Tv>> 
}
```

The interface members are defined below:

Name  |  Purpose
----  |  -------
`next` | This instance method moves the async iterator to the next `Awaitable` position.

### Interface `AsyncKeyedIterator`

This interface supports iteration over the keys and values returned from an [asynchronous generator function](10-expressions.md#yield-operator). It is defined, as follows:

```Hack
interface AsyncKeyedIterator<Tk,Tv> implements AsyncIterator<Tv> {
  public function next(): Awaitable<?tuple<Tk,Tv>> 
}
```

The interface members are defined below:

Name  |  Purpose
----  |  -------
`next` | This instance method moves the async iterator to the next `Awaitable` position.

### Interface `Awaitable`

An instance of this interface is an awaitable object. Such objects are used in support of [asynchronous functions](15-functions.md#asynchronous-functions) and [await](10-expressions.md#await-operator). This interface is defined, as follows:

```Hack
interface Awaitable<T> {
  // unspecified
}
```

The interface members are unspecified.

### Interface `Container`

This interface is a marker for the predefined types `Vector`, `ImmVector`, `Map`, `ImmMap`, `Set`, `ImmSet`, and `Pair` and all array types. This interface is defined, as follows:

```Hack
interface Container<Tv> extends Traversable<Tv> {
}
```

This interface has no members.

### Interface `IMemoizeParam`

Instances of classes that implement this interface can be passed to `serialize_memoize_param` and to functions having the [`__Memoize` attribute](21-attributes.md#Attribute-__Memoize). It is defined, as follows:

```Hack
interface IMemoizeParam implements AsyncIterator<Tv> {
  public function getInstanceKey(): string;
}
```

The interface members are defined below:

Name  |  Purpose
----  |  -------
`getInstanceKey` | Serializes the object to a string that can be used as a dictionary key to differentiate instances of this class.

### Interface `Iterator`

This interface allows instances of an implementing class to be treated
as a collection. This interface is defined, as follows:

```Hack
interface Iterator<Tv> extends Traversable<Tv> {
  public function current(): Tv;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;
}
```

The interface members are defined below:

Name | Purpose
---- | -------
`current` | This instance method returns the element at the current position.
`next` | This instance method moves the current position forward to the next element. From within a `foreach` statement, this method is called after each loop.
`rewind` |  This instance method resets the current position to the first element. From within a `foreach` statement, this method is called once, at the beginning.
`valid` | This instance method checks if the current position is valid. It takes no arguments. It returns a bool value of `true` to indicate the current position is valid; `false`, otherwise. This method is called after each call to [`Iterator::rewind()`](http://docs.hhvm.com/manual/en/iterator.rewind.php) and [`Iterator::next()`](http://docs.hhvm.com/manual/en/iterator.next.php).

### Interface `IteratorAggregate`

This interface allows the creation of an external iterator. This
interface is defined, as follows:

```Hack
interface IteratorAggregate<Tv> extends Traversable<Tv> {
  public function getIterator(): Iterator<Tv>;
}
```
The interface members are defined below:

Name    |   Purpose
----    |   -------
`getIterator` | This instance method retrieves an iterator, which implements `Iterator` or `Traversable`. It throws an `\Exception` on failure.

### Interface `KeyedContainer`

This interface is a marker for the predefined types `Vector`, `ImmVector`, `Map`, `ImmMap`, and `Pair` and all array types. This interface is defined, as follows:

```Hack
interface KeyedContainer<Tk, Tv> extends Container<Tv>, 
  KeyedTraversable<Tk, Tv> {
}
```

This interface has no members.

### Interface `KeyedTraversable`

This interface detects if a class is traversable using `foreach`. This interface is defined, as follows:

```Hack
KeyedTraversable<Tk, Tv> extends Traversable<Tv> {
}
```

This interface has no members.

This interface has no members.

### Interface  `Serializable`

This interface provides support for custom serialization. It is defined,
as follows:

```Hack
interface Serializable {
  public function serialize(): string;
  public function unserialize (string $serialized): void;
}
```

The interface members are defined below:

Name |	Purpose
-----|  -------
`serialize` | This instance method returns a string representation of the current instance. On failure, it returns `null`.
`unserialize` | This instance method constructs an object from its string form designated by `$serialized`.

### Interface `Stringish`

This interface requires an implementing class to provide a conversion to string representation. It is defined, as follows:

```Hack
interface Stringish {
  public function __toString(): string;
}
```

The interface members are defined below:

### Interface `Traversable`

This interface is intended as the base interface for all traversable
classes. This interface is defined, as follows:

```Hack
Traversable<Tv>
{
}
```

