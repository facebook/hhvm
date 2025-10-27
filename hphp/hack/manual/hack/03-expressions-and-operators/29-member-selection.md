# Member Selection

The operator `->` is used to access instance properties and instance
methods on objects.

```hack file:intbox.hack
class IntBox {
  private int $x;

  public function __construct(int $x) {
    $this->x = $x; // Assigning to property.
  }

  public function getX(): int {
    return $this->x; // Accessing property.
  }
}

<<__EntryPoint>>
function main(): void {
  $ib = new IntBox(42);
  $x = $ib->getX(); // Calling instance method.
}
```

## Null Safe Member Access

The operator `?->` allows access to objects that [may be null](/hack/types/nullable-types).

If the value is null, the result is null. Otherwise, `?->` behaves
like `->`.

```hack file:intbox.hack
function my_example(?IntBox $ib): ?int {
  return $ib?->getX();
}
```

Note that arguments are always evaluated, even if the object is
null. `$x?->foo(bar())` will call `bar()` even if `$x` is null.

## XHP Attribute Access

The [operator `->:`](/hack/expressions-and-operators/XHP-attribute-selection) is used for accessing XHP attributes.
