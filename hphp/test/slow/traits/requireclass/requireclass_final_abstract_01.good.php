<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

final abstract class C {
  use T;

  public static function foo(): int { return 42; }
}

trait T {
  require class C;

  public static function bar(): int { return self::foo(); }
}

<<__EntryPoint>>
function main(): void {
  echo C::bar() . "\n";
  echo "done\n";
}
