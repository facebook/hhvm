<?hh

class Foo {
  public static function sm() {
    var_dump(static::class);
    var_dump(self::class);
  }
  public function m() {
    var_dump(static::class);
    var_dump(self::class);
  }
}


class Bar extends Foo {
  public static function sm() {
    var_dump(static::class);
    var_dump(self::class);
    var_dump(parent::class);
  }
  public function m() {
    var_dump(static::class);
    var_dump(self::class);
    var_dump(parent::class);
  }
}

<<__EntryPoint>>
function main() {
  $c = Foo::class;
  $c::sm();
  $o = new $c;
  $o->m();
  $c = Bar::class;
  $c::sm();
  $o = new $c;
  $o->m();
}
