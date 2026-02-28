<?hh

class C {
  public ?string $prop;
}

class :xhp {
  attribute C attr1;
  attribute string attr2;

  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $_,
    string $_,
    int $_,
  ) {
    $c = $attrs['attr'] as C;
    $c->prop = null;
  }
}

function takes_string(string $_): void {}

<<__EntryPoint>>
function main(): void {
  $c = new C();
  <xhp attr1={$c} attr2={$c->prop as nonnull} />;
  takes_string($c->prop);
}
