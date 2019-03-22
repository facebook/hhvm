<?hh

class Blah {
  public static $z;
  public function __call($x, $y) {
    self::$z->hoho();
  }
}

Blah::$z = new Blah();
function main() {

  Blah::$z->whatever();
}

main();;
