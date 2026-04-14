<?hh

class C {
  public function __construct(private int $x) {}
}

function test(classname<C> $cls): void {
  $y = 42;
  new $cls($y);
//         ^ enforcement-at-caret
}
