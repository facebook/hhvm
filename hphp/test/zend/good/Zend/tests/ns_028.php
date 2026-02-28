<?hh

class Foo {
  function __construct() {
    echo "Method - ".__CLASS__."::".__FUNCTION__."\n";
  }
  static function Bar() :mixed{
    echo "Method - ".__CLASS__."::".__FUNCTION__."\n";
  }
}
<<__EntryPoint>> function main(): void {
require "ns_028.inc";
$x = new Foo;
Foo\Bar();
$x = new Foo\Foo;
Foo\Foo::Bar();
\Foo\Bar();
}
