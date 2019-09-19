<?hh // partial

function foo() {}

class Cls {
  public function meth() {}
}

class A {
  public static $arr = meth_caller(Cls::class, 'meth');
}

abstract class B {
  const const_arr = meth_caller(Cls::class, 'meth');
}
