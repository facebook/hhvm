<?hh
namespace test\ns1;

function strlen($x) :mixed{
    return __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
echo strlen("Hello"),"\n";
}
