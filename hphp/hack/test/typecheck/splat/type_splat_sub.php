<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function expectSplatFunAux<T as (mixed...)>(
  (function(int, ...T): void) $f,
  T $tup,
): void {}

function expectSplatFun<T as (mixed...)>(
  (function(int, ...T): void) $f,
  T $tup,
): void {
  // Just test passing a function with a splat parameter to
  // another function that expects the same
  expectSplatFunAux($f, $tup);
  ($f)(2, ...$tup);
}

function none(int $i): void {
}
function one(int $i, num $f): void {
}
function two(int $i, arraykey $s, bool $b): void {
}
function upto_two(int $i, arraykey $s = 'a', bool $b = false): void {
}
function atleast_one(int $i, arraykey $s, num ...$lots): void {}

<<__EntryPoint>>
function test1(): void {
  expectSplatFun(none<>, tuple());
  expectSplatFun(one<>, tuple(3.2));
  expectSplatFun(two<>, tuple("A", false));
  expectSplatFun(upto_two<>, tuple());
  expectSplatFun(upto_two<>, tuple(5));
  expectSplatFun(upto_two<>, tuple(32, true));
  expectSplatFun(atleast_one<>, tuple(45));
  expectSplatFun(atleast_one<>, tuple(45, 2.0));
  expectSplatFun(atleast_one<>, tuple(45, 3.0, 54));
}

// Now do the same with function types
function test2(
  (function(int): void) $none,
  (function(int, num): void) $one,
  (function(int, arraykey, bool): void) $two,
  (function(int, optional arraykey, optional bool): void) $upto_two,
  (function(int, arraykey, num...): void) $atleast_one,
): void {
  expectSplatFun($none, tuple());
  expectSplatFun($one, tuple(3.2));
  expectSplatFun($two, tuple("A", false));
  expectSplatFun($upto_two, tuple());
  expectSplatFun($upto_two, tuple(5));
  expectSplatFun($upto_two, tuple(32, true));
  expectSplatFun($atleast_one, tuple(45));
  expectSplatFun($atleast_one, tuple(45, 2.0));
  expectSplatFun($atleast_one, tuple(45, 3.0, 54));
}

function expectFunWithSplat<T as (mixed...)>(
  (function(int, optional bool, ...T): void) $f,
  ... T $args,
): void {
  ($f)(1, true, ...$args);
}
function expectFunControl((function(int, optional bool): void) $f): void {}

function test3(
  (function(int, bool): void) $g1,
  (function(int): void) $g2,
  (function(int, bool, string): void) $g3,
): void {
  expectFunWithSplat($g2, 23);
  expectFunWithSplat($g3);
}
