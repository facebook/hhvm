<?hh

class :xhp extends XHPTest {
  attribute string attr;
}

class C {
  public ?:xhp $xhp;
}

function xhp_id(:xhp $xhp): :xhp { return $xhp; }

function main(C $c): void {
  if ($c->xhp is nonnull) {
    // This tests that we are not accidentally invalidating refinements as a
    // result of double typecheck against the getAttribute and property-like
    // attribute specification.
    xhp_id($c->xhp)->:attr;
  }
}
