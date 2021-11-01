<?hh

class X {
  public function foo(): void { }
}

trait T1 {
 abstract protected function foo(): void;
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
