<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __Attribute__EAttr implements HH\EnumAttribute {
  public function __construct(public int $i) {}
}

class __Attribute__FAttr implements HH\FunctionAttribute {
  public function __construct(public int $j) {}
}

<<EAttr(1)>>
enum E: int {}

function x(): void {
  $rc = new ReflectionClass("E");
  $rc->getAttributeClass(__Attribute__EAttr::class);
  $rc->getAttributeClass(__Attribute__FAttr::class);
}
