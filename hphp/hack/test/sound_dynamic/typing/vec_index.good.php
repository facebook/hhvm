<?hh

function test_dyn_index(dynamic $d, vec<int> $v1) : void {
  $a = $v1[$d];
  hh_show($a);
  $v1[$d] = 1;
  $v1[$d] = "s";
  $v1[$d] = $d;
}

class C {
  public function __construct(private vec<int> $v1) {}

  public function f(dynamic $d): void {
    $a = $this->v1[$d];
    hh_show($a);
    $this->v1[$d] = 1;
  }
}
