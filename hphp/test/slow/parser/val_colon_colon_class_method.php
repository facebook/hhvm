<?hh

class Foo {
  public static function blah(): void {
    echo __METHOD__ . "\n";
  }

  public static function get_foo(): classname<Foo> {
    return Foo::class;
  }
}

function get_foo(): classname<Foo> {
  return Foo::class;
}

function main(): void {
  $c = Foo::class;
  $c::blah();
  Foo::blah();
  Foo::get_foo()::blah();
  get_foo()::blah();
}


<<__EntryPoint>>
function main_val_colon_colon_class_method() {
main();
}
