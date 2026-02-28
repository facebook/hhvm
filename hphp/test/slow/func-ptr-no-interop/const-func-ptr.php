<?hh

class Cls {
  public static function meth() :mixed{
    return __FUNCTION__;
  }
}

function foo() :mixed{ return __FUNCTION__; }

class A {
  public static $arr = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];

  const arr2 = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}

<<__EntryPoint>>
function main_constant_functions() :mixed{
  var_dump((A::$arr['foo'])());
  var_dump((A::$arr['meth'])());
  var_dump((A::arr2['foo'])());
  var_dump((A::arr2['meth'])());
}
