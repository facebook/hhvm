<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo<reify T> {
  public static function bar<reify Tb>(): void {}

  public function baz<reify Tz>(): void {}
}

function main() : void {
  $f = new Foo<vec>();
  Foo::bar<vec>();
  $f->baz<vec>();
}
