<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function expectSplatFun<T as (mixed...)>(
  (function(int, ...T): void) $f,
  T $tup,
): void {
}

function none(int $i): void {}
function one(int $i, num $f): void {}
function two(int $i, arraykey $s, bool $b): void {}
function upto_two(int $i, arraykey $s = 'a', bool $b = false): void {}
function atleast_one(int $i, arraykey $s, num ...$lots): void {}

// First test with function pointers
function test1(): void {
  // Too many
  expectSplatFun(none<>, tuple(3.2));
  // Not a tuple
  expectSplatFun(none<>, 59);
  // Too few
  expectSplatFun(one<>, tuple());
  // Too many
  expectSplatFun(one<>, tuple(2.3, 4.5));
  // Too few
  expectSplatFun(two<>, tuple("A"));
  // To many
  expectSplatFun(two<>, tuple("A", false, 5));
  // Wrong type in tuple
  expectSplatFun(two<>, tuple("A", 5));
  // Too many
  expectSplatFun(upto_two<>, tuple(2, 'a', true, 45));
  // Wrong type in tuple
  expectSplatFun(upto_two<>, tuple(5, 2.0));
  // Too few
  expectSplatFun(atleast_one<>, tuple());
  // Wrong type in tuple
  expectSplatFun(atleast_one<>, tuple(45, 'a'));
}

// Now do the same with function types
function test2(
  (function(int): void) $none,
  (function(int, num): void) $one,
  (function(int, arraykey, bool): void) $two,
  (function(int, optional arraykey, optional bool): void) $upto_two,
  (function(int, arraykey, num...): void) $atleast_one,
): void {
  // Too many
  expectSplatFun($none, tuple(3.2));
  // Not a tuple
  expectSplatFun($none, 59);
  // Too few
  expectSplatFun($one, tuple());
  // Too many
  expectSplatFun($one, tuple(2.3, 4.5));
  // Too few
  expectSplatFun($two, tuple("A"));
  // To many
  expectSplatFun($two, tuple("A", false, 5));
  // Wrong type in tuple
  expectSplatFun($two, tuple("A", 5));
  // Too many
  expectSplatFun($upto_two, tuple(2, 'a', true, 45));
  // Wrong type in tuple
  expectSplatFun($upto_two, tuple(5, 2.0));
  // Too few
  expectSplatFun($atleast_one, tuple());
  // Wrong type in tuple
  expectSplatFun($atleast_one, tuple(45, 'a'));
}
