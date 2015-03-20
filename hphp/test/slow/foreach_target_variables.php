<?php

class Test {
    public $keyProp, $valProp;
    public static $staticValProp;
}
class Test2 {
    public static $staticKeyProp;
}

$array = [1, 2, 3];

$a = [];
foreach ($array as $a[0] => $a[1]) {
    echo "$a[0] => $a[1],";
}
echo "\n";

$i = 0;
$a = [];
foreach ($array as $a[] => $a[]) {
    echo $a[$i+1], " => ", $a[$i], ",";
    $i += 2;
}
echo "\n";

$obj = new Test;
foreach ($array as $obj->keyProp => $obj->valProp) {
    echo "$obj->keyProp => $obj->valProp,";
}
echo "\n";

foreach ($array as Test2::$staticKeyProp => Test::$staticValProp) {
    echo Test2::$staticKeyProp, " => ", Test::$staticValProp, ",";
}
echo "\n";
