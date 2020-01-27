<?hh // partial

function foo() {}

class Cls {
  public static function meth() {}
}

class A {
  public static $arr = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}

abstract class B {
  const const_arr = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}
