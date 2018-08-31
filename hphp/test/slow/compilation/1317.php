<?php

function f() {
  global $g;
  $g++;
}

<<__EntryPoint>>
function main_1317() {
var_dump((boolean)f(),(int)f(),(double)f(),(string)f());
var_dump((array)f(),(object)f(),(unset)f());
}
