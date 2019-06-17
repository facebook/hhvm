<?hh // partial

class C {
  public function __construct(int $i) {}
}

function f(dynamic $d): void {
  new C($d);
}
