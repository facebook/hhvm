<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}

function testit():string {
  $f = (new Inv((int $x) ==> "a"))->item;
  return $f(2);
}
