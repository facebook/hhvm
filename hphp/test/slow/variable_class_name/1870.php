<?hh

class A {
  const C = 123;
  static public $foo = 456;
  <<__DynamicallyCallable>>
  public static function bar() :mixed{
    return 789;
  }
}

<<__EntryPoint>>
function main_1870() :mixed{
$cls = 'A';

var_dump($cls::C);
 // ClassConstant

var_dump($cls::$foo);
 // StaticMember
$cls::$foo = 'test';
var_dump($cls::$foo);
 // l-value

var_dump($cls::bar());
 // SimpleFunctionCall

$func = 'bar';
var_dump($cls::$func());
}
