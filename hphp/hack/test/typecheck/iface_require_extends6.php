<?hh

class Super {
  public function __construct(string $x) {}
}

interface IMarked {
  require extends Super;
}

class C extends Super implements IMarked {
  public function __construct(int $x) {
    parent::__construct((string) $x);
  }
}
