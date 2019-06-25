<?hh
function f($arg1, &$arg2)
{
    var_dump($arg1++);
    var_dump($arg2++);
}
<<__EntryPoint>> function main(): void {
$a = 7;
$b = 15;

f($a, &$b);

var_dump($a);
var_dump($b);
}
