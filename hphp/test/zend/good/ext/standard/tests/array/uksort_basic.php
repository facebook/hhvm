<?hh
/*
* proto bool uksort ( array &$array, callback $cmp_function )
* Function is implemented in ext/standard/array.c
*/
function cmp($a, $b) {
    if ($a == $b) {
        return 0;
    }
    return ($a < $b) ? -1 : 1;
}
<<__EntryPoint>> function main(): void {
$a = varray[3, 2, 5, 6, 1];
uasort(inout $a, fun("cmp"));
foreach($a as $key => $value) {
    echo "$key: $value\n";
}
}
