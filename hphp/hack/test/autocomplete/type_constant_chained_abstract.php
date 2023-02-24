<?hh

class X {
  const type TFoo = int;
}

abstract class Y {
  abstract const type TBar as X;
}

class Z {
  public function foo(): Y::TBar::TAUTO332 {
  }
}
