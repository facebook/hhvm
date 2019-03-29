<?php

class Test {
    public $keyProp, $valProp;
    public static $staticKeyProp, $staticValProp;
}
class Test2 {
    public static $staticKeyProp, $staticValProp;
}


// Target: Integer array keys
<<__EntryPoint>>
function main_foreach_target_variables() {
$array = [1, 2, 3];

$a = [];
foreach ($array as $a[0] => $a[1]) {
  echo $a[0], " => ", $a[1], ",";
}
echo "\n";

// Target: Append to array
$array = [1, 2, 3];

$i = 0;
$a = [];
foreach ($array as $a[] => $a[]) {
    echo $a[$i+1], " => ", $a[$i], ",";
    $i += 2;
}
echo "\n";

// Target: Non-static properties
$array = [1, 2, 3];

$obj = new Test;
foreach ($array as $obj->keyProp => $obj->valProp) {
    echo "$obj->keyProp => $obj->valProp,";
}
echo "\n";

// Target: Static properties
$array = [1, 2, 3];

foreach ($array as Test2::$staticKeyProp => Test::$staticValProp) {
    echo Test2::$staticKeyProp, " => ", Test::$staticValProp, ",";
}
echo "\n";
}
