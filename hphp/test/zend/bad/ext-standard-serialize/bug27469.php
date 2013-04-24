<?php
$str = 'O:9:"TestClass":0:{}';
$obj = unserialize($str);
var_dump($obj);
echo serialize($obj)."\n";
var_dump($obj);
echo serialize($obj)."\n";
var_dump($obj);
?>