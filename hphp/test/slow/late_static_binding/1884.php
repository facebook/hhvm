<?hh

class S {
  public static function t($arg) :mixed{
    echo $arg;
  }
}
class C {
  public static function d() :mixed{
    echo static::class;
    S::t(static::class);
    echo static::class;
  }
}

<<__EntryPoint>>
function main_1884() :mixed{
C::d();
}
