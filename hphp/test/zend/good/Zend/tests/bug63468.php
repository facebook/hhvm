<?hh
class Foo {
  public function run() {
    return call_user_func(varray['Bar', 'getValue']);
  }

  <<__DynamicallyCallable>> private static function getValue() {
    return 'Foo';
  }
}

class Bar extends Foo {
  <<__DynamicallyCallable>> public static function getValue() {
    return 'Bar';
  }
}

<<__EntryPoint>> function main(): void {
  $x = new Bar;
  var_dump($x->run());
}
