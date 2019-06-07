<?hh
<<__EntryPoint>> function main() {
$test = array();
$test[function(){}] = 1;
$a = array();
$a{function() { }} = 1;
}
