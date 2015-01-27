<?hh
interface I {}
class A implements I {}
class B implements I {}
function x(): I { return a(); }
function y(): I { return b(); }
function z($w) { return $w ? x() : y(); }
function a() { return new A(); }
function b() { return new B(); }
function test() {
  echo get_class(z(true)), "\n";
  echo get_class(z(false)), "\n";
  echo "Done\n";
}
test();
