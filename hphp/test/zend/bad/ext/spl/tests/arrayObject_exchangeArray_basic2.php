<?php
echo "--> exchangeArray(array):\n";
$ao = new ArrayObject();
$ao->exchangeArray(array('key'=>'original'));
var_dump($ao['key']);
var_dump($ao);

echo "\n--> exchangeArray(normal object):\n";
$obj = new stdClass;
$obj->key = 'normal object prop';
$ao->exchangeArray($obj);
var_dump($ao['key']);
var_dump($ao);

echo "\n--> exchangeArray(ArrayObject):\n";
$obj = new ArrayObject(array('key'=>'ArrayObject element'));
$ao->exchangeArray($obj);
var_dump($ao['key']);
var_dump($ao);

echo "\n--> exchangeArray(ArrayIterator):\n";
$obj = new ArrayIterator(array('key'=>'ArrayIterator element'));
$ao->exchangeArray($obj);
var_dump($ao['key']);
var_dump($ao);

echo "\n--> exchangeArray(nested ArrayObject):\n";
$obj = new ArrayObject(new ArrayObject(array('key'=>'nested ArrayObject element')));
$ao->exchangeArray($obj);
var_dump($ao['key']);
var_dump($ao);
?>