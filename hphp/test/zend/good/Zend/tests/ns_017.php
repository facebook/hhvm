<?hh
namespace test\ns1;

function strlen($x) {
    return __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
$x = "strlen";
echo $x("Hello"),"\n";
}
