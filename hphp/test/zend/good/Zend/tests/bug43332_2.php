<?hh
namespace foobar;

class foo {
  public function bar(\self $a) { }
}
<<__EntryPoint>> function main(): void {
$foo = new foo;
$foo->bar($foo); // Ok!
$foo->bar(new stdClass); // Error, ok!
}
