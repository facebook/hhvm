<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// A subtype's named variadic can absorb a compatible explicit named parameter
// when neither parameter has a conflicting attribute.

class C {
  public int $x = 0;
}

// OK: the variadic absorbs the named C parameter in a function type.
function test_absorb_ok(
  (function(named C...): void) $f,
): (function(named C $x): void) {
  return $f;
}

// OK: the same absorption is valid for method overrides.
interface IOk {
  public function m(named C $x): void;
}

class ImplOk implements IOk {
  public function m(named C...): void {}
}
