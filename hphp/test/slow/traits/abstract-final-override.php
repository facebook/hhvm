<?hh

trait T {
    final function foo() {}
}

trait X {
    abstract function foo() ;
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
function main_abstract_final_override() {
  if (isset($g)) {
    include 'abstract-final-override.inc';
  }

  var_dump(new C());
}
