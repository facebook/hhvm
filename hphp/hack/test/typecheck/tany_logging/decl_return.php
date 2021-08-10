<?hh

type Tany<T> = T;

class C {
  /* HH_IGNORE_ERROR[4101] */
  public static function f(): Tany {}

  /* HH_IGNORE_ERROR[4101] */
  public function g(): Tany {}
}

/* HH_IGNORE_ERROR[4101] */
function h(): Tany {}
