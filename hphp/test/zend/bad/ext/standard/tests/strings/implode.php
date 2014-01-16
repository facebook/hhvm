<?php
echo implode(array())."\n";
echo implode('nothing', array())."\n";
echo implode(array('foo', 'bar', 'baz'))."\n";
echo implode(':', array('foo', 'bar', 'baz'))."\n";
echo implode(':', array('foo', array('bar', 'baz'), 'burp'))."\n";
echo $php_errormsg."\n";
?>