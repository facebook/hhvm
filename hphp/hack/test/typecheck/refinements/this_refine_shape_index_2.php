<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class C {
  abstract const type TSchemaShape as shape(...);
  public function __construct(protected this::TSchemaShape $item) { }
}

abstract class B extends C {
}

interface I {
  require extends C;
}
abstract class A extends B {
  abstract const type TSchemaShape as shape('a' => int, ...);
}
final class F {
  const type TSchemaShape = shape('a' => int, 'b' => string);
}
trait TR {
  require implements I;
  public function testit():int {
    if ($this is A || $this is F) {
      return $this->item['a'];
    }
    return 3;
  }
}
