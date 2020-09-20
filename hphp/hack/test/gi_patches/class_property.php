<?hh // partial

const c = 5;

enum E : int {
  X = 0;
}

class X {
  public $a = "string";
  protected $b = "string";
  static private $c = 3;
  public $d = c;
  public $e = E::X;
  private static $f = 3.2;
}
