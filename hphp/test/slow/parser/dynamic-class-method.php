<?hh
class Foo {
  public static function bar($arg) {
    var_dump($arg);
  }
}
function main() {
  Foo::{'bar'}(123);
}

<<__EntryPoint>>
function main_dynamic_class_method() {
main();
}
