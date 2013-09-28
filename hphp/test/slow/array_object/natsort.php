<?php

$array = array("img12.png", "img10.png", "img2.png", "img1.png");

$arr1 = new ArrayObject($array);
$arr2 = clone $arr1;

$arr1->asort();
echo "Standard sorting\n";
print_r($arr1);

$arr2->natsort();
echo "\nNatural order sorting\n";
print_r($arr2);
