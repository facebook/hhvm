<?hh

function foo() :mixed{}

class Cls {
  public static function meth() :mixed{}
}

class A {
  public static $arr = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}

class B {
  const f = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::$arr);
  var_dump(B::f);
  var_dump(A::$arr === B::f);
}
