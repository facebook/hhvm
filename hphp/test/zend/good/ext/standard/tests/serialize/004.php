<?php
ini_set('serialize_precision', 100);

ini_set('precision', 12);

error_reporting (E_ALL);
$a      = array(4);
$str    = serialize($a);
print('Serialized array: '.$str."\n");
$b      = unserialize($str);
print('Unserialized array: ');
var_dump($b);
print("\n");
$str    = serialize(array(4.5));
print('Serialized array: '.$str."\n");
$b      = unserialize($str);
print('Unserialized array: ')   ;
var_dump($b);
?>