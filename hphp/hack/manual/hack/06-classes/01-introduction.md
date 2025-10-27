# Introduction

Classes provide a way to group functionality and state together.

To define a class, use the `class` keyword.

```hack
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
[`new`](/hack/expressions-and-operators/new), e.g. `new Counter();`.
