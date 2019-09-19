<?hh

class Cls {
  public static function meth($_) {
    return __FUNCTION__;
  }
}

function foo($_) { return __FUNCTION__; }

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
  var_dump((A::$arr['foo'])(5));
  var_dump((A::$arr['meth'])(5));
  var_dump((A::arr2['foo'])(5));
  var_dump((A::arr2['meth'])(5));
}
