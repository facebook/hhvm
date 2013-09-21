<?php
/*

It seems the second argument passing is not being correclty checked.
This test is failing due to this wrogn check

*/
$image = imagecreatetruecolor(180, 30);

var_dump(imageantialias($image, 'wrong param')); // 'wrogn param' is converted to true
?>