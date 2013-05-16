<?php
$obj = New SplFileObject(dirname(__FILE__).'/SplFileObject_testinput.csv');
var_dump($obj->fscanf('%s'));
?>