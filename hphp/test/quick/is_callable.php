<?hh

function f() {}
// user function
var_dump(is_callable('f'));
// builtin function
var_dump(is_callable('array_merge'));
// nonexistent function
var_dump(is_callable('asdfjkl'));
echo "\n";

class C {
  public static function foo() {}
  private static function bar() {}
}
// user public static method
var_dump(is_callable(array('C','foo')));
// user private static method
var_dump(is_callable(array('C','bar')));
// nonexistent static method
var_dump(is_callable(array('C','asdfjkl')));
echo "\n";

class D {
  public function foo() {}
  private function bar() {}
}
$obj = new D;
// user public instance method
var_dump(is_callable(array($obj,'foo')));
// user private instance method
var_dump(is_callable(array($obj,'bar')));
// nonexistent instance method
var_dump(is_callable(array($obj,'asdfjkl')));
echo "\n";

interface I {
  public function foo();
}
// user interface method
var_dump(is_callable(array('I','foo')));
// nonexistent interface method
var_dump(is_callable(array('I','asdfjkl')));
echo "\n";

abstract class A {
  public function foo() {}
  abstract public function bar();
}
// user abstract class non-abstract function
var_dump(is_callable(array('A', 'foo')));
// user abstract class abstract function
var_dump(is_callable(array('A', 'bar')));
// nonexistent abstract class method
var_dump(is_callable(array('A', 'asdfghjk')));

