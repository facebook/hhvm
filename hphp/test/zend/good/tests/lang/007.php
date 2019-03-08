<?php
error_reporting(0);
$a = 10;

function Test()
{
	$c = 1;
	Lang007::$b = 5;
	echo TestStatics::$a." ".Lang007::$b." ";
	TestStatics::$a++;
	$c++;
	echo TestStatics::$a." $c ";
}

Test();
echo "$a ".Lang007::$b." $c ";
Test();
echo "$a ".Lang007::$b." $c ";
Test();

abstract final class TestStatics {
  public static $a = 1;
}

abstract final class Lang007 {
  public static $b;
}
