<?hh
require "ns_027.inc";

class Foo {
  function __construct() {
    echo __CLASS__,"\n";
  }
  static function Bar() {
    echo __CLASS__,"\n";
  }
}
<<__EntryPoint>> function main(): void {
$x = new Foo;
Foo::Bar();
$x = new Foo\Bar\Foo;
Foo\Bar\Foo::Bar();
}
