<?hh
namespace Test;
use Test\Foo;
class Foo {}
class Bar {}
use Test\Bar;
<<__EntryPoint>> function main() {
echo "ok\n";
}
