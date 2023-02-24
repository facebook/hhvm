<?hh

class X {
  const type TFoo = int;
}

class Y {
  const type TBar = X;
}

class Z {
  public function foo(): Y::TBar::TAUTO332 {
  }
}
