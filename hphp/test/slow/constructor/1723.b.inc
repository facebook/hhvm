<?hh

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
