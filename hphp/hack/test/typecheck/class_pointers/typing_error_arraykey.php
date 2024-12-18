<?hh

class A {}

class B extends A {
  public function test(class<A> $c0): void {
    $c1 = self::class;
    $c2 = static::class;
    $c3 = parent::class;
    $c4 = A::class;

    dict[$c0 => 0, $c1 => 1, $c2 => 2, $c3 => 3, $c4 => 4];
    Map {$c0 => 0, $c1 => 1, $c2 => 2, $c3 => 3, $c4 => 4};
    keyset[$c0, $c1, $c2, $c3, $c4];
    ImmSet {$c0, $c1, $c2, $c3, $c4};
    Set {$c0, $c1, $c2, $c3, $c4};
  }
}
