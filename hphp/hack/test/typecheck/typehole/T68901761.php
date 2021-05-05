<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class MyExample {
  public function foo(int $this): void {}
}

<<__EntryPoint>>
function my_main(): void {
  (new MyExample())->foo(1);
}
