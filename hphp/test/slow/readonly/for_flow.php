<?hh
class Foo {}
function test_advanced(bool $b): void {
  $x = new Foo();
  $y = new Foo();
  $z = new Foo();
  while ($b) {
    $x = $y;
    $y = $z;
    $z = readonly new Foo();
  }
  $x->prop = 4; // error if loop runs twice
}
