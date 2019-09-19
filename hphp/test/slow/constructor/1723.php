<?hh

if (__hhvm_intrinsics\launder_value(true)) {
  include '1723.inc';
}
class B extends A {
  public function __construct($i, $j, $k) {
    $this->a = $i + $i;
    $this->b = $j + $j;
    $this->c = $k + $k;
  }
  public $a;
  protected $b;
  private $c;
  public $aa = 'aaa';
  protected $bb = 4;
  private $cc = 1.222;
}

<<__EntryPoint>>
function foo() {
  $obj = new B(1, 2, 3);
  var_dump($obj);
}
