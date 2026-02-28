<?hh

interface I {
  abstract const type T as arraykey;
  public function f(): int;
}

class C implements I {
  const type T = int;
  public function f(): this::T {
    return 4;
  }
}

interface I2 extends I {
  require extends C;

  public function f(): this::T;
}
