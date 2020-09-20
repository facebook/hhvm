<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class :xhp implements XHPChild {}

class Inv<T> {
  public function __construct(public T $value) {}
}

function test(:xhp $element): :xhp {
  $element = (new Inv($element))->value;
  return <xhp {...$element}></xhp>;
}
