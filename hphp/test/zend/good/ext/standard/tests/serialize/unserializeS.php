<?php
ini_set('error_reporting', 0);

$str = 'S:'.(100*3).':"'.str_repeat('\61', 100).'"';
$arr = array(str_repeat('"', 200)."1"=>1,str_repeat('"', 200)."2"=>1);

$data = unserialize($str);
var_dump($data);
