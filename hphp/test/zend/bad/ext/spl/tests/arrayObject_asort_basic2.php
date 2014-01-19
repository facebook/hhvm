<?php
/* Prototype  : int ArrayObject::asort()
 * Description: proto int ArrayIterator::asort()
 * Sort the entries by values. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::asort() : basic functionality ***\n";
Class C {
	public $prop1 = 'x';
	public $prop2 = 'z';
	private $prop3 = 'a';
	public $prop4 = 'x';
}

$c = new C;
$ao1 = new ArrayObject($c);
var_dump($ao1->asort());
var_dump($ao1, $c);
?>
===DONE===