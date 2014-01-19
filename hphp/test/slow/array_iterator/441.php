<?php

$arr = array('bar', 'bar', 'bar', 'bar', 'bar', 'bar', 'foo');
function foo() {
  var_dump(__FUNCTION__);
 global $arr;
 $arr[] = 'bar';
}
function bar() {
 var_dump(__FUNCTION__);
 }
reset($arr);
while ($func = each($arr)) {
 $f = $func[1];
 $f();
 }
