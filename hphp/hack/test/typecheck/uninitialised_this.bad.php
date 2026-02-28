<?hh

function boom(): int {
  throw new Exception("boom");
}

class LeakyException extends Exception {
  public function __construct(public C $c) {
    parent::__construct();
  }
}

class C {
  public int $x;

  public function __construct() {
    try {
      $this->x = boom();
    } catch (Exception $e) {
      throw new LeakyException($this);
    }
  }

  public function get(): int {
    return $this->x;
  }
}

<<__EntryPoint>>
function main(): void {
  try {
    $c = new C();
    echo $c->get();
  } catch (LeakyException $e) {
    $e->c->get();
  }
}
