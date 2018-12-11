<?hh // strict

trait T1 {
  public function f(num $i): int {
    return 4;
  }
}

class C {
  use T1;

  public function f(int $k): num = T1::f;
}
