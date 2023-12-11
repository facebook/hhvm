<?hh
/*
* proto bool uksort ( array &$array, callback $cmp_function )
* Function is implemented in ext/standard/array.c
*/
function cmp($a, $b) :mixed{
    if ($a == $b) {
        return 0;
    }
    return ($a < $b) ? -1 : 1;
}
<<__EntryPoint>> function main(): void {
$a = vec[3, 2, 5, 6, 1];
uasort(inout $a, cmp<>);
foreach($a as $key => $value) {
    echo "$key: $value\n";
}
}
