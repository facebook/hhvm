<?hh

abstract final class SetterMagic2 {
  public static $heh;
}

class Heh {
  public function __set($k, $v) {
    var_dump($k, $v);
    test();
  }
}

function test() {
  SetterMagic2::$heh->prop = 3;
}
<<__EntryPoint>> function main(): void {
SetterMagic2::$heh = new Heh;
SetterMagic2::$heh->prop = 2;
var_dump(SetterMagic2::$heh);
}
