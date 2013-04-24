<?php
class Foo1 extends ArrayIterator
{
}
class Foo2 {
}
$x = array(new Foo1(),new Foo2);
$s = serialize($x);
$s = str_replace("Foo", "Bar", $s);
$y = unserialize($s);
var_dump($y);