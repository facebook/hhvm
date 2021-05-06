<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class MyParentClass {
  public function __construct(public string $str) {}
}
function expectString(string $_):void { }
final class MySubClass extends MyParentClass {
  public function __construct(string $str) {
    expectString($this->str);
    parent::__construct($str);
  }
}
<<__EntryPoint>>
function main():void {
  new MySubClass("A");
}
