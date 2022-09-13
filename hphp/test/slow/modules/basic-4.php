<?hh

<<file:__EnableUnstableFeatures("modules")>>

module A;

function test(
  I $x,
  classname<I> $y,
  string $z
) {
  $x::foo(); // pass
  $y::foo(); // pass
  $z::foo(); // error
}

<<__EntryPoint>>
function main() {
  include 'basic-4.inc';
  test(
    conjureC(),
    C::class,
    "C"
  );
}
