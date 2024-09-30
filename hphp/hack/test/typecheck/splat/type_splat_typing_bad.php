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
  // Too many
  applySplatFun(none<>, 3.2);
  // Too few
  applySplatFun(one<>);
  // Too many
  applySplatFun(one<>, 2.3, 4.5);
  // Too few
  applySplatFun(two<>, "A");
  // To many
  applySplatFun(two<>, "A", false, 5);
  // Wrong type
  applySplatFun(two<>, "A", 5);
  // Too many
  applySplatFun(upto_two<>, 2, 'a', true, 45);
  // Wrong type
  applySplatFun(upto_two<>, 5, 2.0);
  // Too few
  applySplatFun(atleast_one<>);
  // Wrong type
  applySplatFun(atleast_one<>, 45, 'a');
}
