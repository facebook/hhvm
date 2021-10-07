<?hh

class Top {
  public function __construct(mixed $m): void {}
}

class Middle extends Top {
  public function __construct(int $i): void {
    parent::__construct($i);
  }
}

trait Badness {
  require extends Top;
  public function __construct(mixed $m): void {
    parent::__construct($m);
  }
}

class Bottom extends Middle {
  use Badness;
}

<<__EntryPoint>>
function main(): void {
  new Bottom("hello"); // exception
}
