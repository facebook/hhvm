<?hh // strict

trait T1 {
  public function f(int $i): int {
    return 4;
  }
}

class C {
  use T1;

  public function f(num $k): num = T1::f;
}
