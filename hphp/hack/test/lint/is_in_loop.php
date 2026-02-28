<?hh

class Foo {}
class Bar extends Foo {}

function test(bool $b, Foo $x): void {
  while ($b) {
    if ($x is Bar) {
      print("x was Bar\n");
    }
    $x = new Bar();
  }
}
