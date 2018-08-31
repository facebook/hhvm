<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function func() {}
}

function takes_callable(@callable $x) {}
function returns_callable($x): @callable { return $x; }

function test() {
  takes_callable(vec['A', 'func']);
  returns_callable(vec['A', 'func']);

  takes_callable(dict[0 => 'A', 1 => 'func']);
  returns_callable(dict[0 => 'A', 1 => 'func']);

  takes_callable(keyset['A', 'func']);
  returns_callable(keyset['A', 'func']);
}


<<__EntryPoint>>
function main_callable_type_hint() {
test();
echo "DONE\n";
}
