<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I { }

final class MyBar implements I {
  public int $x = 0;
}

final class MyFoo implements I {
  public function __construct(private (function(num): readonly MyBar) $getter, private (function(int):MyFoo) $getter2) {}

  public function getBar(bool $b): I {
    $f = $this->getter;
    if ($b) $f = $this->getter2;
    $z = $f(3);
    return $z;
  }
}
