<?php

ArrayIterator441::$arr = array('bar', 'bar', 'bar', 'bar', 'bar', 'bar', 'foo');
function foo() {
  var_dump(__FUNCTION__);

 ArrayIterator441::$arr[] = 'bar';
}
function bar() {
 var_dump(__FUNCTION__);
 }
reset(&ArrayIterator441::$arr);
while ($func = each(&ArrayIterator441::$arr)) {
 $f = $func[1];
 $f();
 }

abstract final class ArrayIterator441 {
  public static $arr;
}
