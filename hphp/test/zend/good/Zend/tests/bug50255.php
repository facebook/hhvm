<?php
<<__EntryPoint>> function main() {
$arr = array('foo' => 'bar');

print "isset\n";
var_dump(isset($arr->foo));
var_dump(isset($arr->bar));
var_dump(isset($arr['foo']));
var_dump(isset($arr['bar']));
}
