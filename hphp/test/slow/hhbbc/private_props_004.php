<?hh

class Other {}

function reffy(&$x) {}

class MVecFinalOps {
  private $s = "this is a string";
  private $o = null;
  private $igen = 42.0;
  private $icell = 2;
  private $igen2 = array();

  public function __construct() {
    $this->o = new Other;
    $this->icell = "asd";
  }

  public function foo() {
    var_dump(isset($this->s));
    var_dump(empty($this->s));
    reffy($this->igen);
    $this->icell += 2;
    $this->icell++;
    $x = array();
    $this->igen2 =& $x;
  }

  public function printer() {
    $s = $this->s;
    $o = $this->o;
    $igen = $this->igen;
    $icell = $this->icell;
    var_dump($s, is_string($s));
    var_dump($o, is_object($o));
    var_dump($igen);
    var_dump($icell);
  }
}

function main() {
  $x = new MVecFinalOps();
  $x->foo();
  $x->printer();
}

main();
