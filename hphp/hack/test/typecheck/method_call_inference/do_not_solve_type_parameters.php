<?hh

interface I<Ti> {
  public function
    test<Tmid as Ti, Tdown as I<Tmid>>(Tdown $box) : Tdown;
}

class X { }

class A {
  public function should_not_break(I<X> $ix) : void {
    $test = $ix->test<_, _>($ix);
  }
}
