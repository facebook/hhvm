<?hh

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

class B {
  const f = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

<<__EntryPoint>>
function main() {
  var_dump(A::$arr);
  var_dump(B::f);
  var_dump(A::$arr === B::f);
}
