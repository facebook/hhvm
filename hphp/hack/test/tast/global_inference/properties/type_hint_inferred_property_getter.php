<?hh //partial

class A {
  public $x;

  public function __construct($x) {
    $this->x = $x;
  }

  public function get() {
    $y = $this->x;
    return $y;
  }
}

function foo(): void {
  $a = new A(1);
  bar($a->get());
}

function bar(int $_): void {}
