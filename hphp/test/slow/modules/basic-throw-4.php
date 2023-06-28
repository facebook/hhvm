<?hh



module A;

function test(
  I $x,
  classname<I> $y,
  string $z
) :mixed{
  $x::foo(); // pass
  try {
    $y::foo(); // pass
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    $z::foo(); // error
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

<<__EntryPoint>>
function main() :mixed{
  include 'basic-4.inc';
  test(
    conjureC(),
    C::class,
    "C"
  );
}
