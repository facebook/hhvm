A *type constraint* on a generic type parameter indicates a requirement that a type must fulfill in order to be accepted as a type
argument for that type parameter. (For example, it might have to be a given class type or a subtype of that class type, or it might
have to implement a given interface.)

A constraint can have one of three forms:
* `T as sometype`, meaning that `T` must be a subtype of `sometype`
* `T super sometype`, meaning that `T` must be a supertype of `sometype`
* `T = sometype`, meaning that `T` must be equivalent to `sometype`. (This is like saying both `T as sometype` *and* `T super sometype`.)

Consider the following example in which function `max_val` has one type parameter, `T`, and that has a constraint, `num`:

```Hack
function max_val<T as num>(T $p1, T $p2): T {
  return $p1 > $p2 ? $p1 : $p2;
}

<<__EntryPoint>>
function main(): void {
  echo "max_val(10, 20) = ".max_val(10, 20)."\n";
  echo "max_val(15.6, -20.78) = ".max_val(15.6, -20.78)."\n";
}
```

Without the `num` constraint, the expression `$p1 > $p2` is ill-formed, as a greater-than operator is not defined for all types. By
constraining the type of `T` to `num`, we limit `T` to being an `int` or `float`, both of which do have that operator defined.

Unlike an `as` constraint, `T super U` asserts that `T` must be a supertype of `U`. This kind of constraint is rather exotic, but solves
an interesting problem encountered when multiple types "collide". Here is an example of how it's used on method `concat` in the library interface
type `ConstVector`:

```Hack no-extract
interface ConstVector<+T> {
  public function concat<Tu super T>(ConstVector<Tu> $x): ConstVector<Tu>;
  // ...
}
```

Consider the case in which we call `concat` to concatenate a `Vector<float>` and a `Vector<int>`. As these have a common supertype, `num`,
the `super` constraint allows the checker to determine that `num` is the inferred type of `Tu`.

Now, while a type parameter on a class can be annotated to require that it is a subtype or supertype of a particular type, for generic parameters
on classes, constraints on the type parameters can be assumed in *any* method in the class. But sometimes some methods want to use some features of
the type parameter, and others want to use some different features, and not all instances of the class will satisfy all constraints. This can be done by
specifying constraints that are *local* to particular methods. For example:

```Hack no-extract
class MyWidget<Telem> {
  public function showIt(): void where Telem as IPrintable { ... }
  public function countIt(): int where Telem as ICountable { ... }
}
```

Constraints can make use of the type parameter itself. They can also make use of generic type parameters on the method. For example:

```Hack
class MyList<T> {
  public function flatten<Tu>(): MyList<Tu> where T = MyList<Tu> {
    throw new Exception('unimplemented');
  }
}
```

Here we might create a list of lists of int, of type `MyList<MyList<int>>`, and then invoke `flatten` on it to get a `MyList<int>`. Here's another example:

```Hack
class MyList<T> {
  public function compact<Tu>(): MyList<Tu> where T = ?Tu {
    throw new Exception('unimplemented');
  }
}
```

A `where` constraint permits multiple constraints supported; just separate the constraints with commas. For example:

```Hack no-extract
class SomeClass<T> {
  function foo(T $x) where T as MyInterface, T as MyOtherInterface
}
```

If a method overrides another method that has declared `where` constraints, it's necessary to redeclare
those constraints, but only if they are actually used by the overriding method. (It's valid, and reasonable, to require less of the overriding method.)
