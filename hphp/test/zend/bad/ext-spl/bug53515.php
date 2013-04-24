<?php

$a = array('a' => 1, 'b'=> true, 'c' => 0, 'd' => null, 'e' => false, 'f' => array());
$o = new ArrayObject($a, ArrayObject::ARRAY_AS_PROPS);

$a['z'] = '';
$a[''] = '';

foreach ($a as $key => $value) {
 echo $key . ': ' . (is_null($value) ? 'null' : @"$value") .
    ' array_key_exists: ' . (array_key_exists($key, $a) ? 'true' : 'false') . 
    ' property_exists: ' . (property_exists($o, $key) ? 'true' : 'false'),"\n";
}

?>