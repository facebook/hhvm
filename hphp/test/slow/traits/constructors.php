<?hh

trait T {
  function x() :mixed{ echo __METHOD__, "\n"; }
  function y() :mixed{ echo __METHOD__, "\n"; }
  function z() :mixed{ echo __METHOD__, "\n"; }
}
trait U {
    function __construct() { echo __METHOD__, "\n"; }
}
class B { use T; }
class X extends B {}
class Y extends B {
  function __construct() { echo __METHOD__, "\n"; }
}
class Z extends B {
    use U;
}

<<__EntryPoint>>
function main_constructors() :mixed{
(new X)->x();
(new Y)->y();
(new Z)->z();
}
