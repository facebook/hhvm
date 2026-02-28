<?hh

function foo($a, $b) :mixed{
}
<<__EntryPoint>> function main(): void {
$funcs = get_defined_functions();
sort(inout $funcs['user']);
var_dump($funcs['user']);
}
