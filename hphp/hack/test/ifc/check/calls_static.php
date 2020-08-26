<?hh

class X {
  public function __construct(
    <<Policied("PUBLIC")>>
    public int $public,
    <<Policied("PRIVATE")>>
    public int $private,
  ) {}

  public static function flow(X $x): void {
    $x->public = $x->private;
  }

  public function testKOSelf(X $x): void {
    self::flow($x); // Flow error
  }

  public function testKOAbsolute(X $x): void {
    X::flow($x); // Flow error
  }

  public function testOK(X $x): void {
    Y::flow($x);
  }
}

class Y extends X {
  public static function flow(X $x): void { }

  public function testKOAbsolute(X $x): void {
    Y::flow($x); // Flow error
  }

  public function testKOParent(X $x): void {
    parent::flow($x); // Flow error
  }

  public function testOK(X $x): void {
    Y::flow($x);
    self::flow($x);
  }
}
