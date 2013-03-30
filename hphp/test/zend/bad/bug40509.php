<?php
function foo()
{
	global $arr;
	
	$c = $arr["v"];
	foreach ($c as $v) {}
}

$arr["v"] = array("a");

var_dump(key($arr["v"]));
foo();
var_dump(key($arr["v"]));
foreach ($arr["v"] as $k => $v) {
	var_dump($k);
}
var_dump(key($arr["v"]));