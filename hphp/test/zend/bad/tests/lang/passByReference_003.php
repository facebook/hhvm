<?php
function passbyVal($val) {
	echo "\nInside passbyVal call:\n";
	var_dump($val);
}

function passbyRef(&$ref) {
	echo "\nInside passbyRef call:\n";
	var_dump($ref);
}

echo "\nPassing undefined by value\n";
passbyVal($undef1[0]);
echo "\nAfter call\n";
var_dump($undef1);

echo "\nPassing undefined by reference\n";
passbyRef($undef2[0]);
echo "\nAfter call\n";
var_dump($undef2)
?>