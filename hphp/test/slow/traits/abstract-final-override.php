<?hh

trait T {
    final function foo() :mixed{}
}

trait X {
    abstract function foo() :mixed;
}

interface I1 {}
interface I2 {}

class A implements I1 {
    use T;
}

abstract class B extends A implements I2 {
    use X;
}

class C extends B {}


<<__EntryPoint>>
function main_abstract_final_override() :mixed{
  if (isset($g)) {
    include 'abstract-final-override.inc';
  }

  var_dump(new C());
}
