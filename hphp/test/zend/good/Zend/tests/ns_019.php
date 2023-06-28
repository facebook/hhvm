<?hh
<<__DynamicallyCallable>>
function foo() :mixed{
    return __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
$x = __NAMESPACE__ . "\\foo";
echo $x(),"\n";
}
