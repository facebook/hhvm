<?hh

class C {
  public function __construct(private int $x) {}
}

function test(): void {
  $y = 42;
  new C($y);
//      ^ enforcement-at-caret
}
