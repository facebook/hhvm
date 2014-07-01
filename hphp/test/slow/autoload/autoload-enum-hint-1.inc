<?hh

// Test autoloading to get an enum type hint
function foo(@Foo $x) {
  var_dump($x);
}
foo(12); // Fine
foo("hi"); // Bogus

// Test autoloading when we reference an enum constant
var_dump(Bar::BAR);

// Test autoloading when we define an enum
enum Quux : Baz {
  BAZ = 100; // this is dubious, but should work in HHVM
}
function quux(@Quux $x) {
  var_dump($x);
}
quux(Quux::BAZ); // Fine
quux("hi"); // Bogus
