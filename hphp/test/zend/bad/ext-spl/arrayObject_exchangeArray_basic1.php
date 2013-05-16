<?php
$ao = new ArrayObject();
$swapIn = array();
$cowRef = $swapIn; // create a copy-on-write ref to $swapIn
$ao->exchangeArray($swapIn);

$ao['a'] = 'adding element to $ao';
$swapIn['b'] = 'adding element to $swapIn';
$ao['c'] = 'adding another element to $ao';

echo "\n--> swapIn:  ";
var_dump($swapIn);

echo "\n--> cowRef:  ";
var_dump($cowRef);

echo "\n--> ao:  ";
var_dump($ao);
?>