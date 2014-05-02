<?php
apc_store('foo', array("foo"));
$a = apc_fetch('foo');
var_dump($a);
sort($a);
var_dump($a);
