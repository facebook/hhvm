<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {}
function test_advanced(bool $b): void {
  $x = new Foo();
  $y = new Foo();
  $z = new Foo();
  for ($i = 0; $i < 5; $i++) {
    $x = $y;
    $y = $z;
    $z = readonly new Foo();
  }
  $x->prop = 4; // error if loop runs twice
}
