<?hh

class Obj { public function yo() { echo "yo\n"; } }

class Foob {
  private static $x;

  public static function heh() {
    if (!self::$x) {
      self::$x = new Obj();
    }
    return self::$x;
  }
}

function main() {
  Foob::heh()->yo();
  Foob::heh()->yo();
}


<<__EntryPoint>>
function main_static_props_002() {
main();
}
