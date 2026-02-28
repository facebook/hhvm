<?hh
namespace test\ns1;

class Foo {
  static function bar() :mixed{
    echo __CLASS__,"\n";
  }
}

use test\ns1\Foo as Bar;
use test\ns1 as ns2;
use test\ns1;
<<__EntryPoint>> function main(): void {
Foo::bar();
\test\ns1\Foo::bar();
Bar::bar();
ns2\Foo::bar();
ns1\Foo::bar();
}
