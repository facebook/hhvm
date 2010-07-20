--TEST--
Constant in Array
--FILE--
<?php
class :foo {
  const bar = 'pass';
}
$foo = array('etc' => :foo::bar);
echo $foo['etc'];
--EXPECT--
pass
