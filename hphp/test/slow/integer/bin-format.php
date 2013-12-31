<?php

echo "simple\n";
var_dump(0b0);
var_dump(0b1);
var_dump(0b1111);
var_dump(0b0000);
var_dump(0b1000);
var_dump(0b0001);

echo "negative simple\n";
var_dump(-0b0);
var_dump(-0b1);
var_dump(-0b1111);

echo  "simple operations\n";
var_dump(0b10 + 2); // int(4)
var_dump(0b10 - 1); // int(1)
var_dump(0b10 * 2); // int(4)
var_dump(0b11 / 2); // float(1.5)

echo  "large\n";
var_dump(0b11111111111111111111111111111111);
var_dump(0b11111111111111111111111111111111+1);
var_dump(0b1111111111111111111111111111111111111111111111111111111111111111);
var_dump(-0b11111111111111111111111111111111);
var_dump(-0b11111111111111111111111111111111-1);
var_dump(-0b1111111111111111111111111111111111111111111111111111111111111111);
var_dump(-0b1111111111111111111111111111111111111111111111111111111111111111 - 1);

// consistent overflow-behaviour - THIS CONTRADICTS ZEND's BEHAVIOUR
//
// As soon as HHVM is changed to "normal" Zend behaviour, these tests
// need to be edited/removed
echo "overflows\n";
var_dump(0b1111111111111111111111111111111111111111111111111111111111111111 + 1);
var_dump(-0b1111111111111111111111111111111111111111111111111111111111111111 - 2);

// make sure 0b overflows like normal integers and 0x hexadecimal integer
echo "overflow consistency\n";
$ofBin = 0b1111111111111111111111111111111111111111111111111111111111111111 + 1;
$ofInt = 9223372036854775807 + 1;
$ofHex = 0xFFFFFFFFFFFFFFFF + 1;
$ufBin = -0b1111111111111111111111111111111111111111111111111111111111111111 - 2;
$ufInt = -9223372036854775807 - 2;
$ufHex = -0xFFFFFFFFFFFFFFFF - 2;

var_dump($ofBin === $ofInt);
var_dump($ofBin === $ofHex);
var_dump($ufBin === $ufInt);
var_dump($ufBin === $ufHex);

