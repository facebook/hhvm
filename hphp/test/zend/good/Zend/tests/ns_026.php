<?hh
namespace Foo;

class Foo {
  function __construct() {
      echo "Method - ".__CLASS__."::".__FUNCTION__."\n";
  }
  static function Bar() :mixed{
      echo "Method - ".__CLASS__."::".__FUNCTION__."\n";
  }
}

function Bar() :mixed{
  echo "Func   - ".__FUNCTION__."\n";
}
<<__EntryPoint>> function main(): void {
$x = new Foo;
\Foo\Bar();
$x = new \Foo\Foo;
\Foo\Foo::Bar();
\Foo\Bar();
Foo\Bar();
}
