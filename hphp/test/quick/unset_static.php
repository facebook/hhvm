<?hh
class Foo {
  public static $baz = 32;
}
<<__EntryPoint>> function main(): void {
unset(Foo::$baz);
}
