<?hh

abstract final class GetterMagic { public static $heh; }
error_reporting(-1);

class Heh {
  protected $prop;

  public function __get($k) {
    var_dump($k);
    test();
  }
}

function test() {
  var_dump(GetterMagic::$heh->prop);
}

GetterMagic::$heh = new Heh;
var_dump(GetterMagic::$heh->prop);
var_dump(GetterMagic::$heh);
