--TEST--
XHP Class Attributes
--FILE--
<?php
class :foo {
  public static $bar;
  const etc = 1;
}
(:foo::$bar = 2);
echo :foo::etc;
echo :foo::$bar;
--EXPECT--
12
