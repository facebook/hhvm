<?hh

// Test autoloading to get an enum type hint
function foo(<<__Soft>> Foo $x) {
  var_dump($x);
}

// Test autoloading when we define an enum
enum Quux : Baz {
  BAZ = 100; // this is dubious, but should work in HHVM
}
function quux(<<__Soft>> Quux $x) {
  var_dump($x);
}

function test() {
foo(12); // Fine
foo("hi"); // Bogus

// Test autoloading when we reference an enum constant
var_dump(Bar::BAR);

quux(Quux::BAZ); // Fine
quux("hi"); // Bogus
}
