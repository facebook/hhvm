<?php
$obj = New SplFileObject(dirname(__FILE__).'/SplFileObject_testinput.csv');
$obj->fwrite();
$obj->fwrite('6,6,6',25,null);
?>