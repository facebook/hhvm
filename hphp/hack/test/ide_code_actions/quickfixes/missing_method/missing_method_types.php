<?hh

interface IFoo {
  public function bar<T>(int $x, shape('y' => vec<int>) $s, shape(...) $_, IFoo $other, dynamic $d, mixed $m, (float, num) $tuple, T $t, Vector<null> $v, ?arraykey $ak, nothing $_, ?nonnull $_, int ...$args): void;
}

class Foo implements IFoo {
                   // ^ at-caret
  public function otherMethod(): void {}

}
