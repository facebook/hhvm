<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class MyClass {
  public static function foo(): void {}
}

trait MyTrait {
  public function foo(): void {}
}

class MyChild extends MyClass {
  use MyTrait;
} // fatals when loading the file

<<__EntryPoint>>
function main():void {
  new MyChild();
}
