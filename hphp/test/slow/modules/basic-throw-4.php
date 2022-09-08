<?hh

<<file:__EnableUnstableFeatures("modules")>>

module A;

function test(
  I $x,
  classname<I> $y,
  string $z
) {
  $x::foo(); // pass
  try {
    $y::foo(); // error
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    $z::foo(); // error
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
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
