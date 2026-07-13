<?hh
function f($arg1, inout $arg2)
:mixed{
    $__t1 = $arg1; $arg1++; var_dump($__t1);
    $__t2 = $arg2; $arg2++; var_dump($__t2);
}
<<__EntryPoint>> function main(): void {
$a = 7;
$b = 15;

f($a, inout $b);

var_dump($a);
var_dump($b);
}
