<?hh

function expect_arraykey(arraykey $ak): void {}

class Contain<T as arraykey> {
  private function __construct(private T $t) {}
  public function get(): T {
    return $this->t;
  }
}

function f<Tx as arraykey>(Contain<Tx> $c): void {
  hh_show($c);
  $t = $c->get();
  hh_show($t);
  expect_arraykey($t);
}
