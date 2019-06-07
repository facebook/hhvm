<?hh
namespace test;

function foo() {
    return __FUNCTION__;
}
<<__EntryPoint>> function main() {
$x = __NAMESPACE__ . "\\foo";
echo $x(),"\n";
}
