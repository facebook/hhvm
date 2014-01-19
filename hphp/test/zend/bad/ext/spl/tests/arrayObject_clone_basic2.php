<?php
class C { }

$c = new C;
$ao1 =  new ArrayObject($c);
$c->p1 = 'new prop added to c before clone';

$ao2 = clone $ao1;

$c->p2 = 'new prop added to c after clone';
$ao1['new.ao1'] = 'new element added to ao1';
$ao2['new.ao2'] = 'new element added to ao2';
var_dump($c, $ao1, $ao2);
?>