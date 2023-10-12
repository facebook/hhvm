<?hh // strict

class Base {
  const type T = this;
  const type U = Base;
}

class C extends Base {
  public function test(
    TypeStructure<this::T> $x,
    TypeStructure<this::U> $y,
  ): void {
    hh_show($x['classname']);
    hh_show($y['classname']);
  }
}
