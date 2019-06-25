<?hh
namespace test\ns1;

function strlen($x) {
    return __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
$x = "test\\ns1\\strlen";
echo $x("Hello"),"\n";
}
