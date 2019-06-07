<?hh
namespace test\ns1;

class Exception {
}
<<__EntryPoint>> function main() {
$x = "test\\ns1\\Exception";
echo \get_class(new $x),"\n";
}
