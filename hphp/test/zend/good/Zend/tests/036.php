<?hh
<<__EntryPoint>> function main(): void {
$test = darray[];
$test[function(){}] = 1;
$a = darray[];
$a{function() { }} = 1;
}
