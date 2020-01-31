<?hh //partial

class A {
  public $x;

  public function __construct($x) {
    $this->x = $x;
  }

  // Also make $this-x covariant, to make sure it is solved
  // to lower bounds.
  public function getX() {
    return $this->x;
  }
}

function foo(): void {
  $_ = new A(1);
}
