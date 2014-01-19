<?php

function f($a) {
 $a[] = $a;
 var_dump($a);
 }
f(false);
