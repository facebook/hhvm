<?php
class T {
	static $a = array(false=>"false", true=>"true");
}
print_r(T::$a);

echo "\n----------\n";

const X = 0;
const Y = 1;
class T2 {
	static $a = array(X=>"false", Y=>"true");
}
print_r(T2::$a);
