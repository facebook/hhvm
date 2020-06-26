<?hh

abstract class X {
  <<__Memoize>>
  public abstract function f(): void;
}

final class Y extends X {
  public function f(): void {}
}
