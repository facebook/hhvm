<?hh
class Foo {
  public $bar = "bat";
}
<<__EntryPoint>> function main(): void {
$foo = new Foo;
var_dump($foo);

$bar = (int)$foo;
var_dump($bar);

$baz = (float)$foo;
var_dump($baz);
}
