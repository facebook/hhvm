<?hh
// Ignored in strict mode (used for analysis only)
function foo($x) : unicorns {
  return "42\n";
}
class Foo {
  function foo($x) : bunnies {
    return function():rainbows use ($x) {
      return foo($x);
    };
  }
}
$v = Foo::foo(1);
echo $v();
