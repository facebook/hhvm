<?php

$array = array(
  'IMG0.png',
  'img12.png',
  'img10.png',
  'img2.png',
  'img1.png',
  'IMG3.png'
);

$arr1 = new ArrayObject($array);
$arr2 = clone $arr1;

$arr1->asort();
echo "Standard sorting\n";
print_r($arr1);

$arr2->natcasesort();
echo "\nNatural order sorting (case-insensitive)\n";
print_r($arr2);
