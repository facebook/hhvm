<?php
error_reporting(0);
$a = 10;

function Test()
{
	static $a=1;

	$c = 1;
	Lang007::$b = 5;
	echo "$a ".Lang007::$b." ";
	$a++;
	$c++;
	echo "$a $c ";
}

Test();	
echo "$a ".Lang007::$b." $c ";
Test();	
echo "$a ".Lang007::$b." $c ";
Test();

abstract final class Lang007 {
  public static $b;
}
