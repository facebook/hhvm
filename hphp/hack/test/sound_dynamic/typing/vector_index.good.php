<?hh

function test_dyn_index(dynamic $d, Vector<int> $v2) : void {
  $a = $v2[$d];
  hh_expect_equivalent<~int>($a);
  $v2[$d] = 1;
}

class C {
  public function __construct(private Vector<int> $v2) {}
  public function f(dynamic $d): void {
    $a = $this->v2[$d];
    hh_expect_equivalent<~int>($a);
    $this->v2[$d] = 1;
  }
}
