<?hh

class Blah {
  public static $z;
  public function __call($x, $y) {
    self::$z->hoho();
  }
}

<<__EntryPoint>> function main(): void {
  Blah::$z = new Blah();
  Blah::$z->whatever();
}
