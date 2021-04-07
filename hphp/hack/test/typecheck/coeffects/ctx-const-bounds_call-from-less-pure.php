<?hh

abstract class Base {
  abstract const ctx C super [write_props];

  public function m()[this::C]: void {}

  public static function caller(Base $b)[write_props]: void {
    $b->m(); // OK: [write_props] <: lower([this::C])
  }

  public function covarGood((function()[this::C]: void) $fn): void {
    takes_fn_write_props($fn); // OK
    // (function()[this::C]: void) <: (function()[write_props]: void)
    // write_prop <: this::C  <=> write_props <: lower(this::C)
  }

  public function contravarBad()[]: (function()[this::C]: void) {
    return ()[write_props] ==> {}; // ERROR: write_props >/: this::C
  }

  public function covar2Good()[]: (function()[write_props]: void) {
    return ()[this::C] ==> {}; // OK: this::C >: write_props
  }
}

function takes_fn_write_props(
  (function()[write_props]: void) $fn
)[]: void {}
