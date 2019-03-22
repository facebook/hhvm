<?hh

abstract final class IncdecMagic4 {
  public static $lol;
}

class Whatever {
  public function __get($name) {
    var_dump($name); return IncdecMagic4::$lol;
  }
}

IncdecMagic4::$lol = "asd";

function main() {
  $l = new Whatever();
  $l->blah++;
  var_dump($l);
}

main();
var_dump(IncdecMagic4::$lol);
