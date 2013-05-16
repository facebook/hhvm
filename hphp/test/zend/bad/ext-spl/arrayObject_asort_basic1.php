<?php
/* Prototype  : int ArrayObject::asort()
 * Description: proto int ArrayIterator::asort()
 * Sort the entries by values. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::asort() : basic functionality ***\n";

$ao1 = new ArrayObject(array(4,2,3));
$ao2 = new ArrayObject(array('a'=>4,'b'=>2,'c'=>3));
var_dump($ao1->asort());
var_dump($ao1);
var_dump($ao2->asort('blah'));
var_dump($ao2);
var_dump($ao2->asort(SORT_NUMERIC));
var_dump($ao2);
?>
===DONE===