<?hh

class X {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PUBLIC")>>
    public int $public,
    <<__Policied("PRIVATE")>>
    public int $private,
  ) {}

  <<__InferFlows>>
  public static function flow(X $x): void {
    $x->public = $x->private;
  }

  <<__InferFlows>>
  public function testKOSelf(X $x): void {
    self::flow($x); // Flow error
  }

  <<__InferFlows>>
  public function testKOAbsolute(X $x): void {
    X::flow($x); // Flow error
  }

  <<__InferFlows>>
  public function testOK(X $x): void {
    Y::flow($x);
  }
}

class Y extends X {
  <<__InferFlows>>
  public static function flow(X $x): void { }

  <<__InferFlows>>
  public function testKOAbsolute(X $x): void {
    Y::flow($x); // Flow error
  }

  <<__InferFlows>>
  public function testKOParent(X $x): void {
    parent::flow($x); // Flow error
  }

  <<__InferFlows>>
  public function testOK(X $x): void {
    Y::flow($x);
    self::flow($x);
  }
}
