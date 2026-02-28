<?hh
namespace test\ns1;

function strlen($x) :mixed{
    return __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
$x = "strlen";
echo \HH\dynamic_fun($x)("Hello"),"\n";
}
