<?hh

abstract final class IncdecMagic5 {
  public static $lol;
}

class Whatever {
  public function __get($name) {
    var_dump($name); return IncdecMagic5::$lol;
  }
  public function __set($k, $v) {}
}

IncdecMagic5::$lol = "asd";

function main() {
  $l = new Whatever();
  $l->blah++;
  var_dump($l);
}

main();
var_dump(IncdecMagic5::$lol);
