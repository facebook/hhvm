<?hh
function valRef($x, inout $y) :mixed{
    var_dump($x, $y);
    $x = 'changed.x';
    $y = 'changed.y';
}

function refVal(inout $x, $y) :mixed{
    var_dump($x, $y);
    $x = 'changed.x';
    $y = 'changed.y';
}

<<__EntryPoint>> function main(): void {
echo "\n\n-- Val, Ref --\n";
$a = 'original.a';
valRef($a, inout $a);
var_dump($a);

echo "\n\n-- Ref, Val --\n";
$b = 'original.b';
refVal(inout $b, $b);
var_dump($b);
}
