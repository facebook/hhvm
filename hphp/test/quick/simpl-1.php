<?php

function mult1($x) {
  return $x * 1;
}

var_dump(mult1(false));
var_dump(mult1(null));
var_dump(mult1(1));
var_dump(mult1(1.5));
var_dump(mult1(""));
var_dump(mult1("1"));
