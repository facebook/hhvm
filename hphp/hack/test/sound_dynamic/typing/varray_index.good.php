<?hh

function test_dyn_index(dynamic $d, varray<int> $v1) : void {
  $a = $v1[$d];
  hh_expect_equivalent<int>($a);
  $v1[$d] = 1;
  $v1[$d] = "s";
  $v1[$d] = $d;
}

class C {
  public function __construct(private varray<int> $v1) {}

  public function f(dynamic $d): void {
    $a = $this->v1[$d];
    hh_expect_equivalent<int>($a);
    $this->v1[$d] = 1;
  }
}
