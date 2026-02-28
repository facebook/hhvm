# Null

The `null` type has only one possible value, the value `null`.

You can use the `null` type when refining with `is`.

```hack
function number_or_default(?int $x): int {
  if ($x is null) {
    return 42;
  } else {
    return $x;
  }
}
```

See [nullable types](/hack/types/nullable-types) for a discussion of `?T`
types.

The `null` type is also useful when writing generics. Suppose you want
to define a generic interface with a 1-argument function, but some
instances don't need an argument.

```hack file:traversefrom.hack
// A toy interface that allows you to iterate over something,
// setting a start point.
interface TraverseFrom<Tv, Ti> {
  public function startAt(Ti $_): Traversable<Tv>;
}
```

You can use `null` to define a class implementing this interface,
making it clear that you don't care about the argument to `startAt`.


```hack file:traversefrom.hack
class TraverseIntsFromStart implements TraverseFrom<int, null> {
  public function __construct(private vec<int> $items) {}

  public function startAt(null $_): Traversable<int> {
    return $this->items;
  }
}
```
