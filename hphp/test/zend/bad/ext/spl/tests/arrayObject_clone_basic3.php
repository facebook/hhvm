<?php
class C {
	public $p = 'C::p.orig';
}

$wrappedObject = new C;
$innerArrayObject = new ArrayObject($wrappedObject);

$outerArrayObject =  new ArrayObject($innerArrayObject);

$wrappedObject->dynamic1 = 'new prop added to $wrappedObject before clone';
$clonedOuterArrayObject = clone $outerArrayObject;
$wrappedObject->dynamic2 = 'new prop added to $wrappedObject after clone';

$innerArrayObject['new.iAO'] = 'new element added $innerArrayObject';
$outerArrayObject['new.oAO'] = 'new element added to $outerArrayObject';
$clonedOuterArrayObject['new.coAO'] = 'new element added to $clonedOuterArrayObject';

var_dump($wrappedObject, $innerArrayObject, $outerArrayObject, $clonedOuterArrayObject);
?>