<?php

class c1
{
	const ZERO = 0;
	const ONE = 1;
	const MAX = PHP_INT_MAX;
	public static $a1 = array(self::ONE => 'one');
	public static $a2 = array(self::ZERO => 'zero');
	public static $a3 = array(self::MAX => 'zero');
}


c1::$a1[] = 1;
c1::$a2[] = 1;
c1::$a3[] = 1;

var_dump(c1::$a1);
var_dump(c1::$a2);
var_dump(c1::$a3);
?>
