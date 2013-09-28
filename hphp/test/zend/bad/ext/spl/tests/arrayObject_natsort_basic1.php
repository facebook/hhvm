<?php
/* Prototype  : int ArrayObject::natsort()
 * Description: proto int ArrayIterator::natsort()
 Sort the entries by values using "natural order" algorithm. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::natsort() : basic functionality ***\n";

$ao1 = new ArrayObject(array('boo10','boo1','boo2','boo22','BOO5'));
$ao2 = new ArrayObject(array('a'=>'boo10','b'=>'boo1','c'=>'boo2','d'=>'boo22','e'=>'BOO5'));
var_dump($ao1->natsort());
var_dump($ao1);
var_dump($ao2->natsort('blah'));
var_dump($ao2);
?>
===DONE===