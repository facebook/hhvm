<?php
class MyArrayObject extends ArrayObject {
	private $priv1 = 'secret1';
	public $pub1 = 'public1';
} 

$ao = new ArrayObject(array(1,2,3));
$ao->p = 1;
var_dump($ao);

$ao = new ArrayObject(array(1,2,3), ArrayObject::STD_PROP_LIST);
$ao->p = 1;
var_dump($ao);

$ao = new MyArrayObject(array(1,2,3));
var_dump($ao);

$ao = new MyArrayObject(array(1,2,3), ArrayObject::STD_PROP_LIST);
var_dump($ao);
?>