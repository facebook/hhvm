<?php
function doForeach($array)
{
    foreach ($array as $k => $v) {
        // do stuff
    }
}

$foo = array('foo', 'bar', 'baz');
var_dump(key($foo));
doForeach($foo);
var_dump(key($foo));
foreach ($foo as $k => $v) {
	var_dump($k);
}
var_dump(key($foo));
