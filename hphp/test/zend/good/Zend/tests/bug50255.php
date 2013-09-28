<?php

$arr = array('foo' => 'bar');

print "isset\n";
var_dump(isset($arr->foo));
var_dump(isset($arr->bar));
var_dump(isset($arr['foo']));
var_dump(isset($arr['bar']));
print "empty\n";
var_dump(empty($arr->foo));
var_dump(empty($arr->bar));
var_dump(empty($arr['foo']));
var_dump(empty($arr['bar']));

?>