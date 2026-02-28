<?hh

class P {
  public function foo(named int $x = 1, named int $y, bool $z): void {}
}

class Child0 extends P {
  // ok
  public function foo(bool $z, named int $y, named int $x = 1): void {}
}

class Child2 extends P {
  // ok
  public function foo(named int $y = 1, named int $x = 1, bool $z): void {}
}

class Child3 extends P {
  // error: too few
  public function foo(bool $z): void {}
}
class Child4 extends P {
  // error: too many
  public function foo(named int $x, named int $y, bool $z, named int $ww): void {}
}
class Child5 extends P {
  // error: x should be optional
  public function foo(named int $x, named int $y, bool $z): void {}
}
class Child6 extends P {
  // multiple errors
  public function foo(named int $x, named int $y, bool $z, named int $extra): void {}
}
