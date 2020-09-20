<?hh

class C {
  public function __construct(
    <<Policied("PRIVATE")>>
    public int $private,
  ) { }
}

class D extends C {
  public function __construct(
    <<Policied("PUBLIC")>>
    public int $public,
    public int $private,
  ) {
    parent::__construct($private);
  }
}

function test(): void {
  $d = new D(42,24);

  $d->public = $d->private;
}
