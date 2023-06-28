<?hh
function f($arg1, inout $arg2)
:mixed{
    var_dump($arg1++);
    var_dump($arg2++);
}
<<__EntryPoint>> function main(): void {
$a = 7;
$b = 15;

f($a, inout $b);

var_dump($a);
var_dump($b);
}
