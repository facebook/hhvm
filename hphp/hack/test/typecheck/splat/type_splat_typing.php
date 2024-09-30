<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function applySplatFun<T as (mixed...)>(
  (function(int, ...T): void) $f,
  ... T $tup,
): void {
  //($f)(2, ...$tup);
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
  applySplatFun(none<>);
  applySplatFun(one<>, 3.2);
  applySplatFun(two<>, "A", false);
  applySplatFun(upto_two<>);
  applySplatFun(upto_two<>, 5);
  applySplatFun(upto_two<>, 32, true);
  applySplatFun(atleast_one<>, 44);
  applySplatFun(atleast_one<>, 45, 2.0);
  applySplatFun(atleast_one<>, 45, 3.0, 54);
}
