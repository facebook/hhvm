<?php

$e0 = new Exception('0');
$e1 = new Exception('1');
$e2 = new Exception('2');
$e1->setPrevious($e2);
$e0->setPrevious($e1);

$eA = new Exception('A');
$eB = new Exception('B');
$eC = new Exception('C');
$eB->setPrevious($eC);
$eA->setPrevious($eB);

$e0->setPreviousChain($eA);

$proper_order = '012ABC';
$actual_order = $e0->getMessage();

$cur = $e0;
while ($cur->getPrevious() !== null) {
  $cur = $cur->getPrevious();
  $actual_order .= $cur->getMessage();
}

$result = $actual_order === $proper_order ? 'pass' : 'fail';

echo "{$result}\n";
