<?hh // strict

trait T1 {
  public function f(num $i): num {
    return 4;
  }
}

class C {
  use T1;

  public function f(int $k): int = T1::f;
}
