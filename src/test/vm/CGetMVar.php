<?

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

