<?hh

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PRIVATE")>>
    public int $private,
  ) { }
}

class D extends C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PUBLIC")>>
    public int $public,
    public int $private,
  ) {
    parent::__construct($private);
  }
}

<<__InferFlows>>
function test(): void {
  $d = new D(42,24);

  $d->public = $d->private;
}
