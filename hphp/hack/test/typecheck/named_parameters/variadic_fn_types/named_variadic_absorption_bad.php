<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// A subtype's named variadic can absorb an explicit named parameter from its
// supertype, but it must accept the same parameter attributes as well as its
// type. Each case below should report an attribute mismatch.

class C {
  public int $x = 0;
}

// ERROR: a mutable variadic cannot absorb a readonly named parameter.
function test_absorb_readonly(
  (function(named C...): void) $f,
): (function(named readonly C $x): void) {
  return $f;
}

// ERROR: the same readonly mismatch must fail for method overrides.
interface IReadonly {
  public function m(named readonly C $x): void;
}

class ImplReadonly implements IReadonly {
  public function m(named C ...): void {}
}

// ERROR: an optional named parameter can still be passed, so the subtype's
// variadic must also accept readonly values for that name.
interface IOptionalReadonly {
  public function m(named readonly ?C $x = null): void;
}

class ImplOptionalReadonly implements IOptionalReadonly {
  public function m(named ?C...): void {}
}

class D implements IDisposable {
  public function __dispose(): void {}
}

// ERROR: a variadic without <<__AcceptDisposable>> cannot absorb a named
// parameter that accepts a disposable value.
interface IDispo {
  public function m(<<__AcceptDisposable>> named D $x): void;
}

class ImplDispo implements IDispo {
  public function m(named mixed...): void {}
}

//
