<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T1 {
  public static function oops(): void {}
}

trait T2 {
  public function oops(): void {}
}

class B {
  use T1;
}

class C extends B {
  use T2;
}

<<__EntryPoint>>
function main():void {
  C::oops();
}
