<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __Attribute__CAttr implements HH\ClassAttribute {
  public function __construct(public int $i) {}
}

class __Attribute__FAttr implements HH\FunctionAttribute {
  public function __construct(public int $j) {}
}

<<CAttr(1)>>
class C {}

function x(): void {
  $rc = new ReflectionClass("C");
  $rc->getAttributeClass(__Attribute__CAttr::class);
  $rc->getAttributeClass(__Attribute__FAttr::class);
}
