<?php

function a() {
  $ar1 = array(10, 100, 100, 0);
  $ar2 = array(1, 3, 2, 4);
  array_multisort(&$ar1, &$ar2);
  var_dump($ar1);
  var_dump($ar2);
}

function b() {
  $ar0 = array("10", 11, 100, 100, "a");
  $ar1 = array(1, 2, "2", 3, 1);
  $asc = SORT_ASC;
  $string = SORT_STRING;
  $numeric = SORT_NUMERIC;
  $desc = SORT_DESC;
  array_multisort(&$ar0,
                  &$asc, &$string, &$ar1,
                  &$numeric, &$desc);
  $ar = array(
    $ar0,
    $ar1,
  );
  var_dump($ar);
}

function c() {
  $array = array("Alpha", "atomic", "Beta", "bank");
  $array_lowercase = array_map("strtolower", $array);
  $asc = SORT_ASC;
  $string = SORT_STRING;
  array_multisort(&$array_lowercase,
                  &$asc, &$string, &$array);
  var_dump($array);
}



<<__EntryPoint>>
function main_array_multisort() {
a();
b();
c();
}
