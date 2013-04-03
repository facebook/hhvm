<?php

$lambda1 = function () {
	echo "Hello World!\n";
};

$lambda2 = function ($x) {
	echo "Hello $x!\n";
};

var_dump(is_callable($lambda1));
var_dump(is_callable($lambda2));
$lambda1();
$lambda2("Universe");
call_user_func($lambda1);
call_user_func($lambda2, "Universe");

echo "Done\n";
?>