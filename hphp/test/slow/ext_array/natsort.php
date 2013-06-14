<?php

$array1 = array("img12.png", "img10.png", "img2.png", "img1.png");
$array2 = $array1;
sort($array1);
var_dump($array1);

natsort($array2);
var_dump($array2);
