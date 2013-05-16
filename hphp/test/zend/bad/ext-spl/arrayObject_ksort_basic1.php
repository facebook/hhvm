<?php
/* Prototype  : int ArrayObject::ksort()
 * Description: proto int ArrayIterator::ksort()
 * Sort the entries by key. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::ksort() : basic functionality ***\n";
$ao1 = new ArrayObject(array(4,2,3));
$ao2 = new ArrayObject(array('b'=>4,'a'=>2,'q'=>3, 99=>'x'));
var_dump($ao1->ksort());
var_dump($ao1);
var_dump($ao2->ksort('blah'));
var_dump($ao2);
var_dump($ao2->ksort(SORT_STRING));
var_dump($ao2);
?>
===DONE===