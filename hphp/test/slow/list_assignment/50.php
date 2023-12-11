<?hh

class X {
  static public $val = vec[1,2,3];
  function foo() :mixed{
    list($a, $b) = self::$val;
    var_dump($a, $b);
  }
}

<<__EntryPoint>>
function main_50() :mixed{
$x = new X;
$x->foo();
X::$val = null;
}
