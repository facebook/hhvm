<?php

function a() {
  $transport = array("foot", "bike", "car", "plane");
  var_dump(pos($transport));
  var_dump(next($transport));
  var_dump(pos($transport));
  var_dump(prev($transport));
  var_dump(end($transport));
  var_dump(pos($transport));
}

function b() {
  $arr = array();
  var_dump(pos($arr));
}

function c() {
  $arr = array(array());
  var_dump(pos($arr));
}

a();
b();
c();
