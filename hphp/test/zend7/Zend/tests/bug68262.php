<?php

class C {
    public $p;
}

$first = new C;
$first->p = 'init';

$clone = clone $first;
$ref =& $first->p;
unset($ref);

$clone = clone $first;
$clone->p = 'foo';

var_dump($first->p);

?>
