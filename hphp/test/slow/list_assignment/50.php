<?hh

class X {
  static public $val = varray[1,2,3];
  function foo() {
    list($a, $b) = self::$val;
    var_dump($a, $b);
  }
}

<<__EntryPoint>>
function main_50() {
$x = new X;
$x->foo();
X::$val = null;
}
