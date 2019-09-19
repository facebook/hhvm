<?hh // partial

function foo() {}

class Cls {
  public static function meth() {}
}

class A {
  public static $arr = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

abstract class B {
  const const_arr = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}
