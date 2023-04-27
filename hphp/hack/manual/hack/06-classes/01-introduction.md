Classes provide a way to group functionality and state together.

To define a class, use the `class` keyword.

```Hack
class Counter {
  private int $i = 0;

  public function increment(): void {
    $this->i += 1;
  }

  public function get(): int {
    return $this->i;
  }
}
```

To create an instance of a class, use
[`new`](../expressions-and-operators/new.md), e.g. `new Counter();`.
