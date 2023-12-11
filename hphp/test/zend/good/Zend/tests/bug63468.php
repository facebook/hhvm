<?hh
class Foo {
  public function run() :mixed{
    return call_user_func(vec['Bar', 'getValue']);
  }

  <<__DynamicallyCallable>> private static function getValue() :mixed{
    return 'Foo';
  }
}

class Bar extends Foo {
  <<__DynamicallyCallable>> public static function getValue() :mixed{
    return 'Bar';
  }
}

<<__EntryPoint>> function main(): void {
  $x = new Bar;
  var_dump($x->run());
}
