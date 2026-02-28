<?hh
class Foo {
  public static function bar($arg) :mixed{
    var_dump($arg);
  }
}
function main() :mixed{
  Foo::{'bar'}(123);
}

<<__EntryPoint>>
function main_dynamic_class_method() :mixed{
main();
}
