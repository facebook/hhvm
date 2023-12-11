<?hh
/*
* proto bool shuffle ( array &$array )
* Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
$numbers = range(1, 20);
echo "*** testing array_shuffle  \n";
$a = vec[];
var_dump(shuffle(inout $a));
var_dump($a);
$a = vec[1];
var_dump(shuffle(inout $a));
var_dump($a);
$a = dict[2 => 1];
var_dump(shuffle(inout $a));
var_dump($a);
$a = dict["a" => 1];
var_dump(shuffle(inout $a));
var_dump($a);
$a = vec[vec[1, 2, 3]];
var_dump(shuffle(inout $a));
var_dump($a);
$a = vec[1, 1, 1, 1];
var_dump(shuffle(inout $a));
var_dump($a);
$arr1 = dict[5 => 1, 6 => 2, 7 => 3, 8 => 9];
$arr2 = dict[5 => 1, 6 => 2, 7 => 3, 8 => 9];
shuffle(inout $arr1);
echo "this should be 0->...." . count(array_diff($arr1, $arr2)) . "\n";
echo "this should be 4->...." . count(array_intersect($arr1, $arr2)) . "\n";
$bigarray = range(1, 400);
shuffle(inout $bigarray);
echo "this should be 400->...." . count($bigarray) . "\n";
echo "*** testing pass by reference  \n";
$original = $bigarray;
shuffle(inout $bigarray);
$diffarray = array_diff_assoc($original, $bigarray);
if (count($diffarray) < 350) {
    // with 400 entries, the probability that 50 entries or more get the same
    // key-> value association should be so close to zero it wont happen in the lifetime of the
    // universe.
    echo "shuffled array seems to be similar to original\n";
    var_dump($original);
    var_dump($bigarray);
} else {
    echo "test passed \n";
}
}
