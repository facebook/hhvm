# Generic Types, Methods, and Functions

## General

Certain types (classes, interfaces, and traits) and their methods can be *parameterized*; that is, their declarations can have one or more placeholder names—called *type parameters*—that are associated with types via *type arguments* when a class is instantiated or a method is called. A type or method having such placeholder names is called a *generic type* or *generic method*, respectively. Top-level functions can also be parameterized giving rise to *generic functions*.

Generics allow programmers to write a class or method with the ability to be parameterized to any set of types, all while preserving type safety.

Consider the following example in which `Stack` is a generic class having one type parameter, `T`:

```Hack
class StackUnderflowException extends Exception {}

class Stack<T> {
  private array<T> $stack;
  private int $stackPtr;

  public function __construct() {
    $this->stackPtr = 0;
    $this->stack = array();
  }

  public function push(T $value): void {
    $this->stack[$this->stackPtr++] = $value;
  }

  public function pop(): T {
    if ($this->stackPtr > 0) {
      return $this->stack[--$this->stackPtr];
    } else {
      throw new StackUnderflowException();
    }
  }
}
```

As shown, the type parameter `T` is used in the declaration of the instance property `$stack`, as the parameter type of the instance method `push`, and as the return type of the instance method `pop`. Note that although `push` and `pop` use the type parameter, they are not themselves generic methods. 

```Hack
function useIntStack(Stack<int> $stInt): void {
  $stInt->push(10);
  $stInt->push(20);
  $stInt->push(30);
  echo 'pop => ' . $stInt->pop() . "\n";
//  $stInt->push(10.5); // rejected as not being type-safe
}
```

The line commented-out, attempts to call push with a non-`int` argument. This is rejected, because `$stInt` is a stack of `int`.

The *arity* of a generic type or method is the number of type parameters declared for that type or method. As such, class `Stack` has arity 1. The Hack library generic container class `Map` implements an ordered, dictionary-style collection. This type has arity 2, and utilizes a key type and a value type, so the type `Map<int, Employee>`, for example, could be used to represent a group of `Employee` objects indexed by an integer employee number.

Here is an example of a generic function, `maxVal`, having one type parameter, `T`:

```Hack
function maxVal<T>(T $p1, T $p2): T {
  return $p1 > $p2 ? $p1 : $p2;
}
```

The function returns the larger of the two arguments passed to it. In the case of the call `maxVal(10, 20)`, given that the type of both arguments is `int`, that is inferred as the type corresponding to the type parameter `T`, and an `int` value is returned. In the case of the call `maxVal(15.6, -20.78)`, `T` is inferred as `float`, while in `maxVal('red', 'green')`, `T` is inferred as `string`.

Type parameters are discussed further in [§§](14-generic-types-methods-and-functions.md#type-parameters), and type arguments are discussed further in [§§](14-generic-types-methods-and-functions.md#type-arguments).

## Type Parameters

**Syntax**
<pre>
<i>generic-type-parameter-list:</i>
  &lt;  <i>generic-type-parameters</i>  ,<sub>opt</sub>  &gt;
<i>generic-type-parameters:</i>
  <i>generic-type-parameter</i>
  <i>generic-type-parameters</i>  ,  <i>generic-type-parameter</i>
<i>generic-type-parameter:</i>
  <i>generic-type-parameter-variance<sub>opt</sub></i>  <i>generic-type-parameter-name</i>  <i>type-constraint<sub>opt</sub></i>
<i>generic-type-parameter-name:</i>
  <i>name</i>
<i>generic-type-parameter-variance:</i>
  +
  -
</pre>

*name* is defined in [§§](09-lexical-structure.md#names) and *type-constraint* is described in [§§](05-types.md#general).

**Constraints**

Within a *generic-type-parameter-list*, *name*s must be distinct.

All *name*s must begin with the letter T.

A *name* used in a *generic-type-parameter* of a method must not be the same as that used in a *generic-type-parameter* for an enclosing class, interface, or trait.

*generic-type-parameter-variance* must not be present in a *function-definition* ([§§](15-functions.md#function-definitions)). 

The *type-specifier* in *generic-type-constraint* must not be `this` or `?this`.

**Semantics**

A type parameter is a placeholder for a type that is supplied when the generic type is instantiated or the generic method or function is invoked.

A type parameter is a compile-time construct. At run-time, each type parameter is matched to a run-time type that was specified by a type argument. Therefore, a type declared with a type parameter will, at run-time, be a closed generic type ([§§](14-generic-types-methods-and-functions.md#open-and-closed-generic-types)), and execution involving a type parameter uses the actual type that was supplied as the type argument for that type parameter.

The *name* of a type parameter is visible from its point of definition through the end of the type, method, or function declaration on which it is defined. However, the *name* does not conflict with a *name* of the same spelling used in non-type contexts (such as the names of a class constant, an attribute, a method, an enum constant, or a namespace).

Generic type constraints are discussed in [§§](14-generic-types-methods-and-functions.md#type-constraints).

*generic-type-parameter-variance* indicates the variance for that parameter: `+` for covariance, `- `for contravariance. If *generic-type-parameter-variance* is omitted, covariance is assumed.


**Examples**

In the following case, class `Vector` has one type parameter, `Tv`. Method `map` also has one type parameter, `Tu`.

```Hack
final class Vector<Tv> implements MutableVector<Tv> {
  …
  public function map<Tu>((function(Tv): Tu) $callback): Vector<Tu> { … }
}
```

In the following case, class `Map` has two type parameters, `Tk` and `Tv`. Method `zip` has one, `Tu`.

```Hack
final class Map<Tk, Tv> implements MutableMap<Tk, Tv> {
  …
  public function zip<Tu>(Traversable<Tu> $iter): Map<Tk, Pair<Tv, Tu>> { … }
}
```

In the following case, function `maxValue` has one type parameter, `T`.

```Hack
function maxValue<T>(T $p1, T $p2): T { … }
```

## Type Constraints

A *type-constraint* ([§§](05-types.md#general)) in a *generic-type-parameter* indicates a requirement that a type must fulfill in order to be accepted as a type argument for a given type parameter. (For example, it might have to be a given class type or a subtype of that class type, or it might have to implement a given interface.)

Consider the following example in which class `Complex` has one type parameter, `T`, and that has a constraint:

```Hack
class Complex<T as num> {
  private T $real;
  private T $imag;
  public function __construct(T $real, T $imag) {
    $this->real = $real;
    $this->imag = $imag;
  }
  public static function add(Complex<T> $z1, Complex<T> $z2): Complex<T> {
    return new Complex($z1->real + $z2->real, $z1->imag + $z2->imag);
  }
  …
  public function __toString(): string {
    if ($this->imag < 0.0) {                                                       
      return "(" . $this->real . " - " . (-$this->imag) . "i)";
    }
    …
  }
}
```

Without the `num` constraint, a number of errors are reported, including the following: 
* The `return` statement in method add performs arithmetic on a value of unknown type `T`, yet arithmetic isn't defined for all possible type arguments.
* The `if` statement in method `__toString` compares a value of unknown type `T` with a `float`, yet such a comparison isn't defined for all possible type arguments.
* The return statement in method `__toString` negates a value of unknown type `T`, yet such an operation isn't defined for all possible type arguments. Similarly, a value of unknown type `T` is being concatenated with a `string`.

The following code creates `float` and `int` instances, respectively, of class `Complex`:

```Hack
$c1 = new Complex(10.5, 5.67);
echo "\$c1 + \$c2 = " . Complex::add($c1, $c2) . "\n";
$c3 = new Complex(5, 6);
echo "\$c3 + \$c4 = " . Complex::add($c3, $c4) . "\n";
```

## Type Arguments

**Syntax**
<pre>
<i>generic-type-argument-list:</i>
  &lt;  <i>generic-type-arguments</i>  ,<sub>opt</sub>  &gt;
<i>generic-type-arguments:</i>
  <i>generic-type-argument</i>
  <i>generic-type-arguments</i>  ,  <i>generic-type-argument</i>
<i>generic-type-argument:</i>
  <i>type-specifier</i>
  <i>name</i>
</pre>

*name* is defined in [§§](09-lexical-structure.md#names) and *type-specifier* is described in [§§](05-types.md#general).

**Constraints**

Each *generic-type-argument* must satisfy any constraint on the corresponding type parameter ([§§](14-generic-types-methods-and-functions.md#type-parameters)).

**Semantics**

A *generic-type-argument* can be a *type-specifier* or a *name* that is a *type-parameter*. Either way, at runtime, it is used in place of the corresponding type parameter. A *generic-type-argument* can either be open or closed ([§§](14-generic-types-methods-and-functions.md#open-and-closed-generic-types)).

**Examples**

```Hack
final class Pair<Tv1, Tv2> implements ConstVector<mixed> {
  …
  public function getIterator(): KeyedIterator<int, mixed> { … }
  public function keys(): Vector<int> { … }
  public function toMap(): Map<int, mixed> { … }
  public function zip<Tu>(Traversable<Tu> $iter): Vector<Pair<mixed, Tu>> {…}
}
```

In this case, the type specifiers `ConstVector<mixed>`, `KeyedIterator<int, mixed>`, `Vector<int>`, `Map<int, mixed>`, `Traversable<Tu>`, `Vector<Pair<mixed, Tu>>`, and `Tu`, and are type arguments.

## Open and Closed Generic Types

A type parameter is introduced in the corresponding type, method, or function declaration. All other uses of that type parameter occur in *type-specifiers* ([§§](05-types.md#general)) for the declaration of properties, function parameters, and function returns. Each such use can be classified as follows: An *open generic type* is a type that contains one or more type parameters; a *closed generic type* is a type that is not an open generic type.

At run-time, all of the code within a generic type, method, or function declaration is executed in the context of the closed generic type that was created by applying type arguments to that generic declaration. Each type parameter within the generic type, method, or function is associated to a particular run-time type. The run-time processing of all statements and expressions always occurs with closed generic types, and open generic types occur only during compile-time processing.

Two closed generic types are the same type if they are created from the same generic type declaration, and their corresponding type arguments have the same type.

Consider the following:

```Hack
final class Vector<Tv> implements MutableVector<Tv> {
  …
  public function zip<Tu>(Traversable<Tu> $it): Vector<Pair<Tv, Tu>> { … }
}
```

The type parameter `Tv` is introduced in the declaration of the generic type `Vector`. That type parameter is then used in the type-specifiers `MutableVector<Tv>` and `Vector<Pair<Tv, Tu>>`, both of which are open generic types. The type parameter `Tu` is introduced in the declaration of the generic function `zip`. That type parameter is then used in the type-specifiers `Traversable<Tu>` and `Vector<Pair<Tv, Tu>>`, both of which are open generic types.

In the following case:

```Hack
final class Pair<Tv1, Tv2> implements ConstVector<mixed> {
  …
  public function getIterator(): KeyedIterator<int, mixed> { … }
  public function keys(): Vector<int> { … }
  public function toMap(): Map<int, mixed> { … }
  public function zip<Tu>(Traversable<Tu> $iter): Vector<Pair<mixed, Tu>> {…}
}
```

the type specifiers `Traversable<Tu>`, `Tu`, and `Vector<Pair<mixed, Tu>>` are all open generic types, while the type specifiers `ConstVector<mixed>`, `KeyedIterator<int, mixed>`, `Vector<int>`, and `Map<int, mixed>` are all closed generic types.

Static properties specified in a generic type are properties of an open generic type. Type arguments of a static are not associated with a particular run-time type, and thus it is an error to have a static property within a generic type.

Consider the following case:

```Hack
final class Foo<T> {
  public static T $x;
}

function main(): void {
  $i = new Foo(4);
  $s = new Foo("Hi");
}
```

Since the static `$x` is part of the open generic type, there is no way to bind `$x` to a particular type (in this case an `int` or `string`).

## Type Inferencing Revisited

See [§§](05-types.md#type-inferencing) for an introduction to type inferencing.

The examples in this section use the generic class `Stack` defined in [§§](14-generic-types-methods-and-functions.md#general).

Some languages require the type arguments associated with a generic class to be specified at instantiation time, using syntax something like new `Stack<int>()` to create a `Stack` of `int`. However, that is **not** permitted in Hack. As such, what is being allocated here is a stack of unknown type. When method `push` is called with an int argument, the implementation infers that the stack can hold values of that type. Then when `push` is called with a `float` argument, the implementation infers that the stack can also hold values of that type. Values of yet other types can also be pushed.

In the following example, the type of the `Stack` designated by `$st` is not fixed (i.e., established) until that `Stack` is made available outside of its creating function, in this case, when the creating function returns to its caller:

```Hack
function f1(): Stack<num> {
  $st = new Stack();
  $st->push(100);       // allows ints to be pushed
  $st->push(10.5);      // allows floats to be pushed
  return $st;           // fixes the type as Stack<num>
}
```

Now consider the following example:

```Hack
function f2(): void {
  $st = new Stack();
  $st->push(100);
  process($st);         // fixes the type as Stack<int>
  $st->push(10.5);      // rejected once stack type fixed
}
```

When the `Stack` is passed to function process, the `Stack`'s type is fixed as `Stack<int>`, and the subsequent attempt to push on a `float` is rejected.

Note that a similar situation occurs with objects created from collection literals ([§§](10-expressions.md#collection-literals)) having no initial values, as in `Vector {}` and `Map {}`.


