<?hh

function thing_get() :mixed{
  return thing::get();
}

final class thing {
  function x() :mixed{ echo "ok\n"; }

  private static $getInstance = null;

  public static function get() :mixed{
    if (self::$getInstance === null) {
      self::$getInstance = new self();
    }
    return self::$getInstance;
  }
}

function go() :mixed{
  $z = thing_get();
  mt_rand();
  return $z->x();
}


<<__EntryPoint>>
function main_singleton_fail() :mixed{
go();
}
