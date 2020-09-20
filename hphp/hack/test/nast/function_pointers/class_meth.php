<?hh

namespace Foo\Bar;

final class Fizz {
  public static function buzz(): void {}
}

function baz(): void {
  // Elaborate the class
  $x = Fizz::buzz<>;

  // Vector is autoimported class
  $x = Vector::fromItems<>;
}
