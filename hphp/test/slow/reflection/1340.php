<?hh

function foo($a, $b) {
}
<<__EntryPoint>> function main(): void {
$funcs = get_defined_functions();
sort(inout $funcs['user']);
var_dump($funcs['user']);
}
