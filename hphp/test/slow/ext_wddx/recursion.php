<?php
$s1 = wddx_serialize_value(function () {});
var_dump($s1);

$a = []; $a[] =& $a;
$s2 = wddx_serialize_value($a);
var_dump($s2);
