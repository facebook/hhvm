<?hh

namespace Foo\Bar;

function qux(): void {}

function baz(): void {
  // Expect this name to be elaborated
  qux<>;
  // is_vec is autoimported function
  $x = is_vec<>;
  // vec is autoimported function
  $x = vec<>;
}
