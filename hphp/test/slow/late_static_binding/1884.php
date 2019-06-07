<?hh

class S {
  public static function t($arg) {
    echo $arg;
  }
}
class C {
  public static function d() {
    echo static::class;
    S::t(static::class);
    echo static::class;
  }
}

<<__EntryPoint>>
function main_1884() {
C::d();
}
