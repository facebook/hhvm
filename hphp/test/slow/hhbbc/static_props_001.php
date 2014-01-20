<?hh

class Obj {}

class Foob {
  private static $x;

  public function heh() {
    if (!self::$x) {
      self::$x = new Obj();
    }
    return self::$x;
  }

  public function why($z) {
    self::${$z} = 2;
  }
}

function main() {
  Foob::heh();
  Foob::why('x');
  Foob::heh();
}

main();

