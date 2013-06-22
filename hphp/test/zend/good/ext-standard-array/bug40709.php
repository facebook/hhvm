<?php
function CommaSeparatedList($a, $b) {
    if($a == null)
        return $b;
    else
        return $a.','.$b;
}

$arr1 = array(1,2,3);
$arr2 = array(1);

echo "result for arr1: ".array_reduce($arr1,'CommaSeparatedList')."\n";
echo "result for arr2: ".array_reduce($arr2,'CommaSeparatedList')."\n";
echo "result for arr1: ".array_reduce($arr1,'CommaSeparatedList')."\n";
echo "result for arr2: ".array_reduce($arr2,'CommaSeparatedList')."\n";

echo "Done\n";
?>