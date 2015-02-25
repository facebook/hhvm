<?hh

class X {
  public function foo() { }
}

trait T1 {
 abstract protected function foo();
}

trait T {
  use T1;
  use T2;
}

trait T2 {
  require extends X;
}

class C extends X {
  use T;
}
