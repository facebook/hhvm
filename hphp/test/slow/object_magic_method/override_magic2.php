<?hh

class ZZ {
  private $asd;

  function __construct() {
    unset($this->asd);
  }

  function __get($z) {
    echo "get\n";
    return new ZZ();
  }

  static function x(ZZ $x) {
    $x->asd->asd->asd->asd->asd->asd->asd = 2;
    echo "ok\n";
  }
}

<<__EntryPoint>>
function main_override_magic2() {
  ZZ::x(new ZZ);
}
