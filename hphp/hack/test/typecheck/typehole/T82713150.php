<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Foo {
  public static mixed $x = null;

  public static function greet(string $_): void {
    echo "in greet\n";
  }
}

<<__EntryPoint>>
function foo(): void {
  echo "Hello world\n";
  $x = 'greet';
  Foo::$x = (int $_) ==> {
    echo "in lambda\n";
  };
  Foo::$x(10); // boom
}
