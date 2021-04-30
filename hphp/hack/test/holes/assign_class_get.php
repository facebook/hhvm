<?hh

class C {
  private static int $prop = 0;
  private static ?int $nullable_prop = null;
  private static (int, string) $pair_prop = tuple(0, '');
  private static Vector<string> $vec_prop = Vector {};

  public function set_prop(string $x): void {
    /* HH_FIXME[4110] */
    self::$prop = $x;
  }

  public function set_prop_intersect((bool & string) $x): void {
    /* HH_FIXME[4110] */
    self::$nullable_prop = $x;
  }

  public function set_prop_union((int | string) $x): void {
    /* HH_FIXME[4110] */
    self::$prop = $x;
  }
  public function set_pair_prop((int, bool) $x): void {
    /* HH_FIXME[4110] */
    self::$pair_prop = $x;
  }

  public function append_vec_prop(bool $x): void {
    /* HH_FIXME[4110] */
    self::$vec_prop[] = $x;
  }

  public function set_pair_prop_fst(bool $x): void {
    /* HH_FIXME[4110] */
    self::$pair_prop[0] = $x;
  }

  public function set_pair_prop_snd(bool $x): void {
    /* HH_FIXME[4110] */
    self::$pair_prop[1] = $x;
  }
}
