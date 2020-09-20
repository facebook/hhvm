<?hh
namespace test;

<<__DynamicallyConstructible>>
class foo {
}
<<__EntryPoint>> function main(): void {
$x = __NAMESPACE__ . "\\foo";
echo \get_class(new $x),"\n";
}
