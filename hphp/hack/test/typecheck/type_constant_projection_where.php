<?hh

abstract class AC {
  abstract const type Tbar;
}

class C {
  public function foo<Tfoo as AC, Tbar>(Tbar $_): void where Tfoo::Tbar = Tbar {}

  public function bar<Tfoo as AC, Tbar>(Tfoo::Tbar $_): void {}
}
