<?php

function a() {
  $os = array("Mac", "NT", "Irix", "Linux");
  var_dump(in_array("Irix", $os));
  var_dump(!in_array("mac", $os));
}

function b() {
  $a = array("1.10", 12.4, 1.13);
  var_dump(!in_array("12.4", $a, true));
  var_dump(in_array(1.13, $a, true));
}

function c() {
  $a = array(array("p", "h"), array("p", "r"), "o");
  var_dump(in_array(array("p", "h"), $a));
  var_dump(!in_array(array("f", "i"), $a));
  var_dump(in_array("o", $a));
}

a();
b();
c();
