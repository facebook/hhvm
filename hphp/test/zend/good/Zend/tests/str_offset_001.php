<?hh
function foo($x) {
    var_dump($x);
}
<<__EntryPoint>> function main(): void {
$str = "abc";
var_dump($str[-1]);
var_dump($str[0]);
var_dump($str[1]);
var_dump($str[2]);
var_dump($str[3]);
var_dump($str[1][0]);
var_dump($str[2][1]);

foo($str[-1]);
foo($str[0]);
foo($str[1]);
foo($str[2]);
foo($str[3]);
foo($str[1][0]);
foo($str[2][1]);
}
