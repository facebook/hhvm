<?php

function a() {
  $ar1 = array(10, 100, 100, 0);
  $ar2 = array(1, 3, 2, 4);
  array_multisort($ar1, $ar2);
  var_dump($ar1);
  var_dump($ar2);
}

function b() {
  $ar = array(
    array("10", 11, 100, 100, "a"),
    array(1, 2, "2", 3, 1)
  );
  array_multisort($ar[0],
                  SORT_ASC, SORT_STRING, $ar[1],
                  SORT_NUMERIC, SORT_DESC);
  var_dump($ar);
}

function c() {
  $array = array("Alpha", "atomic", "Beta", "bank");
  $array_lowercase = array_map("strtolower", $array);
  array_multisort($array_lowercase,
                  SORT_ASC, SORT_STRING, $array);
  var_dump($array);
}

a();
b();
c();

