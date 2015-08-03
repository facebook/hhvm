<?hh // strict

class Foo {
  // We shouldn't crash hhvm when we have a private ctor...
  private function __construct() { }
}

function test(): void {}
