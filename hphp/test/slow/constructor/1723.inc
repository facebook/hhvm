<?hh

class A {
  public function __construct($i, $j, $k) {
    $this->a = $i * $i;
    $this->b = $j * $j;
    $this->c = $k * $k;
  }
  public $a;
  protected $b;
  private $c;
  public $aa = 'aa';
  protected $bb = false;
  private $cc = 1.22;
}

$x = 1;
