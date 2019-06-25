<?hh
namespace Foo;
use \stdClass;
use \stdClass as A;
<<__EntryPoint>> function main(): void {
echo \get_class(new stdClass)."\n";
echo \get_class(new A)."\n";
}
