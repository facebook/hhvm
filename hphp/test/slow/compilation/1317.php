<?php

function f() {

  Compilation1317::$g++;
}

<<__EntryPoint>>
function main_1317() {
var_dump((boolean)f(),(int)f(),(double)f(),(string)f());
var_dump((array)f(),(object)f(),(unset)f());
}

abstract final class Compilation1317 {
  public static $g;
}
