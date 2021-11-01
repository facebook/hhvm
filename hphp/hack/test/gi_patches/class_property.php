<?hh

/* HH_FIXME[2001] */
const c = 5;

enum E : int {
  X = 0;
}

class X {
  /* HH_FIXME[2001] */
  public $a = "string";
  /* HH_FIXME[2001] */
  protected $b = "string";
  /* HH_FIXME[2001] */
  static private $c = 3;
  /* HH_FIXME[2001] */
  public $d = c;
  /* HH_FIXME[2001] */
  public $e = E::X;
  /* HH_FIXME[2001] */
  private static $f = 3.2;
}
