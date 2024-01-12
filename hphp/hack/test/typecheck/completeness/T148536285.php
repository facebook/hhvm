<?hh

class :xhp extends XHPTest {
  attribute string target;
}

class C {
  public ?string $s;
}

function id(string $x): string {
  return $x;
}

function main(C $c): void {
  $c->s as nonnull;
  <xhp target={id($c->s)} />;
}
