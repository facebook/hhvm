<?php

$arr = array(
 "a" => array("b")
);

$it = new RecursiveArrayIterator($arr);
$it = new RecursiveTreeIterator($it);

$it->setPrefixPart(1); // Should throw a warning as setPrefixPart expects 2 arguments

$a = new stdClass();
$it->setPrefixPart($a, 1); // Should throw a warning as setPrefixPart expects argument 1 to be long integer

$it->setPrefixPart(1, $a); // Should throw a warning as setPrefixPart expects argument 2 to be a string


?>
===DONE===