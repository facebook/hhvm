<?hh

function thing_get() {
  return thing::get();
}

final class thing {
  function x() { echo "ok\n"; }

  private static $getInstance = null;

  public static function get() {
    if (self::$getInstance === null) {
      self::$getInstance = new self();
    }
    return self::$getInstance;
  }
}

function go() {
  $z = thing_get();
  mt_rand();
  return $z->x();
}


<<__EntryPoint>>
function main_singleton_fail() {
go();
}
