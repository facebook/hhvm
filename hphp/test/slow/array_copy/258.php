<?php

function f($a) {
 $a[0] = $a;
 var_dump($a);
 }
f(false);
