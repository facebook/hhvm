<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Foo {
  public function __construct(public (function(): void) $f) {}
  public function f(): void { echo "f: method\n"; }

  public static ?(function(int): int) $bar = null;
  <<__DynamicallyCallable>> public static function pred(int $x): int { return $x - 1; }
  public static function bar(int $x): int  { return 2 * $x; }
 }

<<__EntryPoint>>
function main(): void {
  $e = new Foo(() ==> { echo "f: attribute\n"; });
  $e->f(); // f:method
  ($e->f)(); // f:attribute

  Foo::$bar = $x ==> $x + 1; // succ
  $bar = "pred"; //pred
  var_dump((Foo::$bar)(10)); // int(11)
  if (Foo::$bar) {
    var_dump(Foo::$bar(10)); // int(9)
  }
  var_dump(Foo::bar(10)); // int(20)
}
