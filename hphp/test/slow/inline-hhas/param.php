<?php

function my_is_int($x) {
  hh\asm('
    IsTypeL $x Int
    RetC
  ');
}

var_dump(my_is_int(1));
var_dump(my_is_int(1.2));
