--TEST--
haval algorithm (multi-vector, multi-pass, multi-width)
--FILE--
<?php
echo "Empty String\n";
for($pass=3; $pass<=5; $pass++)
	for($bits=128; $bits <= 256; $bits += 32) {
		$algo = sprintf('haval%d,%d',$bits,$pass);
		echo $algo . ': ' . hash($algo,'') . "\n";
	}

echo "\"abc\"\n";
for($pass=3; $pass<=5; $pass++)
	for($bits=128; $bits <= 256; $bits += 32) {
		$algo = sprintf('haval%d,%d',$bits,$pass);
		echo $algo . ': ' . hash($algo,'abc') . "\n";
	}

echo "\"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789\"\n";
for($pass=3; $pass<=5; $pass++)
	for($bits=128; $bits <= 256; $bits += 32) {
		$algo = sprintf('haval%d,%d',$bits,$pass);
		echo $algo . ': ' . hash($algo,'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789') . "\n";
	}

