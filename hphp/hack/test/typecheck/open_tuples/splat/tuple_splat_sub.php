<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function expectVariadicTuple((arraykey...) $tup): void {}
function expectIntAndVariadicTuple((int, arraykey...) $tup): void {}
function expectOptionalIntAndVariadicTuple(
  (optional int, arraykey...) $tup,
): void {}

<<__NoAutoLikes>>
function expectSplatTuple<T as (arraykey...)>((int, ...T) $tup): T {
  expectVariadicTuple($tup);
  expectIntAndVariadicTuple($tup);
  expectOptionalIntAndVariadicTuple($tup);
  throw new Exception("A");
}

function test1(
  (int) $one,
  (int, arraykey) $two,
  (int, optional arraykey) $one_to_two,
  (int, arraykey, int...) $at_least_two,
): void {
  $x1 = expectSplatTuple($one);
  hh_expect_equivalent<()>($x1);
  $x2 = expectSplatTuple($two);
  hh_expect_equivalent<(arraykey)>($x2);
  $x3 = expectSplatTuple($one_to_two);
  hh_expect_equivalent<(optional arraykey)>($x3);
  $x4 = expectSplatTuple($at_least_two);
  hh_expect_equivalent<(arraykey, int...)>($x4);
}

// Check that function subtyping turns into tuple subtyping
// (function(t1',_,tm', ...T'):r') <:
// (function(t1, _, tm, u1, _, un, ...T):r)
//   if
// ti <: ti'
// r' <: r
// and (u1, _, un, ...T) <: T'
function expectSplatFun<Ts as (mixed...)>(
  (function(int, string, ...Ts): void) $_,
): void {}
function testSplatFun<T super (string, mixed...) as (mixed...)>(
  (function(int, ...T): void) $f,
): void {
  expectSplatFun($f);
}

<<__EntryPoint>>
function testMain(): void {
  testSplatFun((int $x, string $y, ...$z) ==> {});
}
