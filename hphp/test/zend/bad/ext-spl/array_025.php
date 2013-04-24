<?php
$obj1 = new ArrayObject(new ArrayObject(array(1,2)));
$s = serialize($obj1);
$obj2 = unserialize($s);

print_r($obj1);
echo "$s\n";
print_r($obj2);
?>