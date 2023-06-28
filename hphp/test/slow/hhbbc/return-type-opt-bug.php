<?hh
interface I {}
class A implements I {}
class B implements I {}
function x(): I { return a(); }
function y(): I { return b(); }
function z($w) :mixed{ return $w ? x() : y(); }
function a() :mixed{ return new A(); }
function b() :mixed{ return new B(); }
function test() :mixed{
  echo get_class(z(true)), "\n";
  echo get_class(z(false)), "\n";
  echo "Done\n";
}

<<__EntryPoint>>
function main_return_type_opt_bug() :mixed{
test();
}
