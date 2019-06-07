<?hh

class X {
  private $priv;
  protected $prot;
  public $pub;
  function __construct($a,$b,$c) {
    $this->priv = $a;
    $this->prot = $b;
    $this->pub = $c;
  }
  function foo() {
 var_dump($this->priv, $this->prot, $this->pub);
 }
}

<<__EntryPoint>>
function main_1548() {
$x = new X(1,2,3);
$s = serialize($x);
$x = unserialize($s);
var_dump($x);
$x->foo();
}
