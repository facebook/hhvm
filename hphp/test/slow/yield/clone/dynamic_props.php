<?php

function gen() { yield 7; yield 8; }
$g = gen();
$g->lol = "whut";
var_dump($g);
$h = clone($g);
var_dump($h);
