<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  private function test(): string {
    return "test was called";
  }
}

class Bar {
  private function blarg(): void {
    $a = new Foo();
    $a->test();
  }
}
