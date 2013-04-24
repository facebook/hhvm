<?php

var_dump(sscanf('one two', '%1$s %2$s'));
var_dump(sscanf('one two', '%2$s %1$s'));
echo "--\n";
sscanf('one two', '%1$s %2$s', $foo, $bar);
var_dump($foo, $bar);
sscanf('one two', '%2$s %1$s', $foo, $bar);
var_dump($foo, $bar);
echo "--\n";
var_dump(sscanf('one two', '%1$d %2$d'));
var_dump(sscanf('one two', '%1$d'));
echo "Done\n";
?>