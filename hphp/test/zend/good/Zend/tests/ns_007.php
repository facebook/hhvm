<?hh
namespace test\ns1;

class Exception {
}
<<__EntryPoint>> function main(): void {
$x = "Exception";
echo \get_class(new $x),"\n";
}
