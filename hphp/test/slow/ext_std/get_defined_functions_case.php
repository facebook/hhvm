<?hh

function UpPer():mixed{}
<<__EntryPoint>> function main(): void {
$funcs = get_defined_functions()['user'];
sort(inout $funcs);
var_dump($funcs);
}
