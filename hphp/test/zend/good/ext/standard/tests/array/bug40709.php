<?hh
function CommaSeparatedList($a, $b) {
    if($a == null)
        return $b;
    else
        return $a.','.$b;
}
<<__EntryPoint>> function main(): void {
$arr1 = varray[1,2,3];
$arr2 = varray[1];

echo "result for arr1: ".array_reduce($arr1,fun('CommaSeparatedList'))."\n";
echo "result for arr2: ".array_reduce($arr2,fun('CommaSeparatedList'))."\n";
echo "result for arr1: ".array_reduce($arr1,fun('CommaSeparatedList'))."\n";
echo "result for arr2: ".array_reduce($arr2,fun('CommaSeparatedList'))."\n";

echo "Done\n";
}
