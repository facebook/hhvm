<?php

class test {
	static public $ar = array();
}

var_dump(test::$ar);

test::$ar[] = 1;

var_dump(test::$ar);

echo "Done\n";
?>
