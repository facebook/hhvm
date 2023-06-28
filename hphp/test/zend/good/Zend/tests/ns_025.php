<?hh
namespace Foo\Bar;

class Foo {
  function __construct() {
      echo __CLASS__,"\n";
  }
  static function Bar() :mixed{
      echo __CLASS__,"\n";
  }
}
<<__EntryPoint>> function main(): void {
$x = new Foo;
Foo::Bar();
$x = new \Foo\Bar\Foo;
\Foo\Bar\Foo::Bar();
}
