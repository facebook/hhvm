<?hh

class Cls {
  public static function meth() {
    return __FUNCTION__;
  }
}

function foo() { return __FUNCTION__; }

class A {
  public static $arr = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];

  const arr2 = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

<<__EntryPoint>>
function main_constant_functions() {
  var_dump((A::$arr['foo'])());
  var_dump((A::$arr['meth'])());
  var_dump((A::arr2['foo'])());
  var_dump((A::arr2['meth'])());
}
