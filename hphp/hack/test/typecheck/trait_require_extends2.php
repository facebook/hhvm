<?hh

trait T1 {
  require extends X;
  <<__Override>>
  final protected function foo(): void { }
}

trait T2 {
  require extends X;
}

class X  {
  protected function foo(): void { }
}

class A extends X {
  use T1;
  use T2;
  protected function foo(): void { }
}
