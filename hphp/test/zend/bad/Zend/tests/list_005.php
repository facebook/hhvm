<?php

$a = "foo";

list($a, $b, $c) = $a;

var_dump($a, $b, $c);

print "----\n";

$a = 1;

list($a, $b, $c) = $a;

var_dump($a, $b, $c);

print "----\n";

$a = new stdClass;

list($a, $b, $c) = $a;

var_dump($a, $b, $c);

print "----\n";

$a = array(1, 2, 3);

list($a, $b, $c) = $a;

var_dump($a, $b, $c);

?>