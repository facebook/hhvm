<?hh

class C {
  public function __construct(private vec<int> $v1) {}

  public function f(dynamic $d): void {
    $this->v1[$d] = "s";
    $this->v1[$d] = $d;
  }
}
