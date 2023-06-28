<?hh

// Note that this test uses ConstantFunctions to intentionally change
// the behavior in repo mode.

function foo($x = 1, $y = 1) :mixed{ return $x * $y; }

class X {

  private static $barF = 1;
  static function bar($x = 1, $y = 1) :mixed{
    return self::$barF++ * $x + $y;
  }
}

function bar() :mixed{
  return foo();
}


<<__EntryPoint>>
function main_constant_functions() :mixed{
var_dump(bar());
var_dump(foo(3, 4));
var_dump(X::bar());
var_dump(X::bar());
var_dump(X::bar(2, 2));
var_dump(X::bar(2, 2));
}
