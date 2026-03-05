<?hh

abstract class Foo {
  abstract const type TFoo as this::TFoo;
  public function test(this::TFoo $x): void {
    hh_expect<int>($x);
  }
}

abstract class Bar {
  abstract const type TBar1 as this::TBar2::TBar1;
  abstract const type TBar2 as Qux with { type TBar1 = this::TBar1; };
  public function test(this::TBar1 $x): void {
    hh_expect<int>($x);
  }
}

abstract class Qux {
  abstract const type TBar1;
}
