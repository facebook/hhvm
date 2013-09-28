<?php
/* Prototype  : int ArrayObject::natcasesort()
 * Description: proto int ArrayIterator::natcasesort()
 Sort the entries by values using case insensitive "natural order" algorithm. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::natcasesort() : basic functionality ***\n";

$ao1 = new ArrayObject(array('boo10','boo1','boo2','boo22','BOO5'));
$ao2 = new ArrayObject(array('a'=>'boo10','b'=>'boo1','c'=>'boo2','d'=>'boo22','e'=>'BOO5'));
var_dump($ao1->natcasesort());
var_dump($ao1);
var_dump($ao2->natcasesort('blah'));
var_dump($ao2);
?>
===DONE===