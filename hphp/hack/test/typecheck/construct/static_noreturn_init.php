<?hh //strict

class C {
  private int $x;

  /* HH_FIXME[4336] */
  private static function invariant_violation(): noreturn {
  }

  public function __construct(bool $cond) {
    if ($cond) {
      self::invariant_violation();
    } else {
      $this->x = 4;
    }
  }
}
