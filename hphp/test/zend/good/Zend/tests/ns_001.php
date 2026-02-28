<?hh
namespace test\ns1;

class Foo {

  function __construct() {
    echo __CLASS__,"\n";
  }

  function bar() :mixed{
    echo __CLASS__,"\n";
  }

  static function baz() :mixed{
    echo __CLASS__,"\n";
  }
}
<<__EntryPoint>> function main(): void {
$x = new Foo;
$x->bar();
Foo::baz();
$y = new \test\ns1\Foo;
$y->bar();
\test\ns1\Foo::baz();
}
