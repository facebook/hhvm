<?hh

interface I {
  const v = 42;
}
trait T implements I {}
class C {
  use T;
  const v = 42;
}
class D extends C {}

<<__EntryPoint>>
function main_const_from_trait_interface() :mixed{
$c = new D;
var_dump($c is I);
}
