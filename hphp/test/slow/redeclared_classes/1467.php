<?php

function f($i) {
  $j = 1;
  var_dump($j);
  if ($i == 1) {
    class p {
      public $data1;
    }
    class c extends p {
    }
    function r() {
 echo "r1\n";
 }
  }
 else {
    class p {
      public $data2;
    }
    class c extends p {
    }
    function r() {
 echo "r2\n";
 }
  }
}
if ($i == 1) {
  class p {
    public $data1;
  }
  class c extends p {
  }
  function r() {
 echo "r3\n";
 }
}
f(1);
$obj = new p();
var_dump($obj);
$obj = new c();
var_dump($obj);
r();
