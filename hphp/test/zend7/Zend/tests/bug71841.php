<?php
$z = unserialize('O:1:"A":0:{}');
var_dump($z->e.=0);
var_dump(++$z->x);
var_dump($z->y++);

$y = array(PHP_INT_MAX => 0);
var_dump($y[] .= 0);
var_dump(++$y[]);
var_dump($y[]++);
?>
