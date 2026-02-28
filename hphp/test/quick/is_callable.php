<?hh

function f() :mixed{}

class C {
  public static function foo() :mixed{}
  private static function bar() :mixed{}
}

class D {
  public function foo() :mixed{}
  private function bar() :mixed{}
}

interface I {
  public function foo():mixed;
}

abstract class A {
  public function foo() :mixed{}
  abstract public function bar():mixed;
}

<<__EntryPoint>> function main(): void {
// user function
var_dump(is_callable('f'));
// builtin function
var_dump(is_callable('array_merge'));
// nonexistent function
var_dump(is_callable('asdfjkl'));
echo "\n";

// user public static method
var_dump(is_callable(vec['C','foo']));
// user private static method
var_dump(is_callable(vec['C','bar']));
// nonexistent static method
var_dump(is_callable(vec['C','asdfjkl']));
echo "\n";

$obj = new D;
// user public instance method
var_dump(is_callable(vec[$obj,'foo']));
// user private instance method
var_dump(is_callable(vec[$obj,'bar']));
// nonexistent instance method
var_dump(is_callable(vec[$obj,'asdfjkl']));
echo "\n";

// user interface method
var_dump(is_callable(vec['I','foo']));
// nonexistent interface method
var_dump(is_callable(vec['I','asdfjkl']));
echo "\n";

// user abstract class non-abstract function
var_dump(is_callable(vec['A', 'foo']));
// user abstract class abstract function
var_dump(is_callable(vec['A', 'bar']));
// nonexistent abstract class method
var_dump(is_callable(vec['A', 'asdfghjk']));
}
