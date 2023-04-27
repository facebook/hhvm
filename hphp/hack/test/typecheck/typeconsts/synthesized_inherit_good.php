<?hh

interface I {
  abstract const type T as arraykey;
}

class C implements I {
  const type T = int;
}

trait T implements I {
  require extends C;

  public function foo(): this::T {
    return 3;
  }
}
