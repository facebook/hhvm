<?php
/* Prototype  : int ArrayObject::ksort()
 * Description: proto int ArrayIterator::ksort()
 * Sort the entries by key. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::ksort() : basic functionality ***\n";
Class C {
	public $x = 'prop1';
	public $z = 'prop2';
	public $a = 'prop3';
	private $b = 'prop4';
}

$c = new C;
$ao1 = new ArrayObject($c);
var_dump($ao1->ksort());
var_dump($ao1, $c);
?>
===DONE===