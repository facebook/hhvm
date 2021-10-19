<?hh
class Foo {}
function test_advanced(bool $b): void {
  $w = readonly new Foo();
  $x = new Foo();
  $y = new Foo();
  $z = new Foo();
  do {
    $w = new Foo(); // $w becomes mutable permanantly
    $x = $y;
    $y = $z;
    $z = readonly new Foo();
  } while ($b);
  $w->prop = 4; // Ok, $w is always mutable here
  $x->prop = 4; // error if loop runs twice
}
