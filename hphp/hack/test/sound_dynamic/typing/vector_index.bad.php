<?hh

function test_dyn_idx(
  dynamic $d,
  Vector<int> $v2,
) : void {
  $v2[$d] = "s";
  $v2[$d] = $d;
}

class C {
  public function __construct(private Vector<int> $v2) {}

  public function f(dynamic $d): void {
    $this->v2[$d] = "s";
    $this->v2[$d] = $d;
  }
}
