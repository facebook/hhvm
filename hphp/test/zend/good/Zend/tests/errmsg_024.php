<?php

class test1 {
	static protected $var = 1;
}

class test extends test1 {
	static $var = 10;
}

echo "Done\n";
?>