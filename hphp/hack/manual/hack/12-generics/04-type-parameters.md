# Type Parameters

A type parameter is a placeholder for a type that is supplied when a generic type is instantiated, or a generic method or function is invoked.

A type parameter is a compile-time construct. At run-time, each type parameter is matched to a run-time type that was specified by a
type argument.

The name of a type parameter is visible from its point of definition through the end of the type, method, or function declaration on
which it is defined. However, the name does not conflict with a name of the same spelling used in non-type contexts (such as the names
of a class constant, an attribute, a method, an enum constant, or a namespace). All type-parameter names *must* begin with the letter T.

In the following case, class `Vector` has one type parameter, `Tv`. Method `map` also has one type parameter, `Tu`.

```hack no-extract
final class Vector<Tv> implements MutableVector<Tv> {
  // ...
  public function map<Tu>((function(Tv): Tu) $callback): Vector<Tu> { ... }
}
```

In the following case, class `Map` has two type parameters, `Tk` and `Tv`. Method `zip` has one, `Tu`.

```hack no-extract
final class Map<Tk, Tv> implements MutableMap<Tk, Tv> {
  // ...
  public function zip<Tu>(Traversable<Tu> $iter): Map<Tk, Pair<Tv, Tu>> { ... }
}
```

In the following case, function `max_value` has one type parameter, `T`.

```hack
function max_value<T>(T $p1, T $p2): T {
  throw new Exception("unimplemented");
}
```

Generic type constraints are discussed in [type constraints](/hack/generics/type-constraints).
