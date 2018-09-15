<?php

$notStr = 123;
try {
	sodium_increment($notStr);
} catch (SodiumException $e) {
	echo $e->getMessage(), "\n";
}

$str = "abc";
$str2 = $str;
sodium_increment($str);
var_dump($str, $str2);

$str = "ab" . ($x = "c");
$str2 = $str;
sodium_increment($str);
var_dump($str, $str2);

$addStr = "\2\0\0";

$notStr = 123;
try {
	sodium_add($notStr, $addStr);
} catch (SodiumException $e) {
	echo $e->getMessage(), "\n";
}

$str = "abc";
$str2 = $str;
sodium_add($str, $addStr);
var_dump($str, $str2);

$str = "ab" . ($x = "c");
$str2 = $str;
sodium_add($str, $addStr);
var_dump($str, $str2);

?>
