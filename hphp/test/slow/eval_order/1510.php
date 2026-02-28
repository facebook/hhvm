<?hh

class Q {
  public $val;
  function __construct($v) {
    $this->val = $v;
  }
  public function blah() :mixed{
    return $this;
  }
}
class A {
  public $v;
  function set($v) :mixed{
    $this->v = $v;
    return $this;
  }
}
function id($x) :mixed{
 return $x;
 }

<<__EntryPoint>>
function main_1510() :mixed{
$x = new Q(0);
$a = id(new A)->set($x);
$x = id(new Q(1))->blah();
var_dump($a);
}
