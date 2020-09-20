<?hh
namespace test\ns1;

<<__DynamicallyConstructible>>
class Exception {
}
<<__EntryPoint>> function main(): void {
$x = "test\\ns1\\Exception";
echo \get_class(new $x),"\n";
}
