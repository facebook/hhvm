<?php

function f() {
  global $g;
  $g++;
}
var_dump((boolean)f(),(int)f(),(double)f(),(string)f());
var_dump((array)f(),(object)f(),(unset)f());
