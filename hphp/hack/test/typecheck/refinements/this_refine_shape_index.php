<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class C {
  abstract const type TSchemaShape as shape(...);
  public function __construct(protected this::TSchemaShape $item) { }
}

interface I {
  require extends C;
}
final class F {
  const type TSchemaShape = shape('a' => int, 'b' => string);
}
trait TR {
  require implements I;
  public function testit():int {
    if ($this is F) {
      return $this->item['a'];
    }
    return 3;
  }
}
