<?hh // strict

class MyClass {
  public int $x = 0;
}

function foo(MyClass $bar): void {
  $bar?->x;
}
