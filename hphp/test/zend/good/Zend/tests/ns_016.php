<?hh
namespace test\ns1;
<<__DynamicallyCallable>>
function strlen($x) :mixed{
    return __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
$x = "test\\ns1\\strlen";
echo $x("Hello"),"\n";
}
