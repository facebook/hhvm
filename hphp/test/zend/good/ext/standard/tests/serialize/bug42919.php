<?hh
namespace Foo;
class Bar {
}
<<__EntryPoint>> function main(): void {
echo \serialize(new Bar) . "\n";
$x = \unserialize(\serialize(new Bar));
echo \get_class($x) . "\n";
}
