<?hh
class Foo {
  static $baz = 32;
}
<<__EntryPoint>> function main(): void {
unset(Foo::$baz);
}
