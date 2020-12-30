<?hh

class C {
  <<__InferFlows>>
  public function __construct(
    public string $file
  ): void {}
}

class D {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("FILE")>>
    public string $file,
    <<__Policied("PUBLIC")>>
    public string $public_file,
    <<__Policied("PUBLIC")>>
    public C $public_c,
  ) {}
}

<<__InferFlows>>
function leak_via_field(D $d): void {
  $c = new C($d->file);
  $d->public_file = $c->file; // FILE flows to PUBLIC
}

<<__InferFlows>>
function leak_via_subtyping(D $d): void {
  $c = new C($d->file);
  $d->public_c = $c; // FILE flows to PUBLIC
}
