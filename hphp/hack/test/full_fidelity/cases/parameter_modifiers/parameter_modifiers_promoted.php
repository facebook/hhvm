<?hh

class C1 {
  public function __construct(public public int $x) {}
}

class C2 {
  public function __construct(public private int $x) {}
}
