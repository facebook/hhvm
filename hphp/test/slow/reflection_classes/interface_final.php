<?hh

interface I {
  function m();
}

class A implements I {
  final function m() { }
}

class B extends A { }

abstract class C extends B implements I { }

class D extends C { }


<<__EntryPoint>>
function main_interface_final() {
$rc = new ReflectionClass('I');
var_dump($rc->getMethods()[0]->isFinal());

$rc = new ReflectionClass('A');
var_dump($rc->getMethods()[0]->isFinal());

$rc = new ReflectionClass('B');
var_dump($rc->getMethods()[0]->isFinal());

$rc = new ReflectionClass('C');
var_dump($rc->getMethods()[0]->isFinal());

$rc = new ReflectionClass('D');
var_dump($rc->getMethods()[0]->isFinal());
}
