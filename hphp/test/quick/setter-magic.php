<?hh

abstract final class SetterMagic {
  public static $heh;
}

error_reporting(-1);

class Heh {
  protected $prop;

  public function __set($k, $v) {
    var_dump($k, $v);
    test();
  }
}

function test() {
  // Note: this is different from zend right now.  Zend drops this set
  // without a notice or fatal.  We are raising a fatal since we can't
  // access the protected property.
  SetterMagic::$heh->prop = 3;
}

SetterMagic::$heh = new Heh;
SetterMagic::$heh->prop = 2;
var_dump(SetterMagic::$heh);
