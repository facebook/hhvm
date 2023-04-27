The type `void` indicates the absence of a value. It is used to declare that a function does *not* return any value. As such, a void function
can contain one or more `return` statements, provided none of them return a value.  Consider the following example:

```Hack
function draw_line(Point $p1, Point $p2): void { /* ... */ }

class Point {
  private float $x;
  private float $y;
  public function __construct(num $x = 0, num $y = 0) {
    $this->x = (float)$x;
    $this->y = (float)$y;
  }
  public function move(num $x = 0, num $y = 0): void {
    $this->x = (float)$x;
    $this->y = (float)$y;
  }
  // ...
}

<<__EntryPoint>>
function main(): void {
  draw_line(new Point(1.2, 3.3), new Point(6.2, -4.5));
}
```

As is often the case, a function like `draw_line` causes something to happen but does not need to return a result or success code, so its return
type is `void`. Likewise, for method `move`.

Here's another example, involving a generic stack type:

```Hack
class Stack<T> {
  // ...
  public function push(T $value): void { /* ... */ }
  // ...
}
```

The act of pushing a value on a stack does not require any value to be returned. However, method `push` could fail due to stack overflow; however,
as that is an extreme situation, the method likely would throw an exception that is only handled when the condition occurs rather than returning a
success/failure code that would have to be checked on every call.
