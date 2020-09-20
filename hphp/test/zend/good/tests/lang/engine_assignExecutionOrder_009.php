<?hh
function f($x) {
    echo "f($x) ";
    return $x;
}
<<__EntryPoint>> function main(): void {
echo "Function call args:\n";
var_dump(f($i=0) < f(++$i));
var_dump(f($i=0) <= f(++$i));
var_dump(f($i=0) > f(++$i));
var_dump(f($i=0) >= f(++$i));

echo "\nArray indices:\n";
$a = darray[1 => darray[], 3 => darray[]];
$a[1][2] = 0;
$a[3][4] = 1;
$i=0;
var_dump($a[$i=1][++$i] < $a[++$i][++$i]);
var_dump($a[$i=1][++$i] <= $a[++$i][++$i]);
var_dump($a[$i=1][++$i] > $a[++$i][++$i]);
var_dump($a[$i=1][++$i] >= $a[++$i][++$i]);
}
