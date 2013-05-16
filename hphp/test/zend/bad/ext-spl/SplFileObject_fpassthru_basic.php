<?php
$obj = New SplFileObject(dirname(__FILE__).'/SplFileObject_testinput.csv');
$obj->fpassthru();
?>