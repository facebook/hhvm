<?hh

type Tany<T> = T;

class C {
  /* HH_IGNORE_ERROR[4101] */
  public static function static_method_param(Tany $_, Tany $_): void {}

  /* HH_IGNORE_ERROR[4101] */
  public function method_param(Tany $_): void {}
}

/* HH_IGNORE_ERROR[4101] */
function fx_param(Tany $_): void {}

/* HH_IGNORE_ERROR[4101] */
function inout_param(inout Tany $_): void {}

/* HH_IGNORE_ERROR[4101] */
function variadic_param(Tany ...$_): void {}
