<?hh

class Foo {
  public static function bar() {
    var_dump(Foo::class);
    var_dump(foo::class);
    var_dump(self::class);
  }
}

class Baz extends Foo {
  public function qux() {
    var_dump(parent::class);
    var_dump(static::class);
  }
}


<<__EntryPoint>>
function main_reflective_constant() {
Foo::bar();
$b = new Baz;
$b->qux();
}
