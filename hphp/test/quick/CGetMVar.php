<?

class V { const X = 10; }
function &values() {
  static $VALUES = array(
    'X'                        => V::X,
  );
  return $VALUES;
}
function foo() { var_dump(values()['X']); }
foo();
foo();

class T {
  public $var;
  function f() {
    echo $this->var - 3;
  }
}

$a = 33;
$t = new T();
$t->var = &$a;
$t->f();
$a = 10000;
$t->f();
