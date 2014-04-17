<?php
class t1{
  var $v1 = 2.1;

  var $v2 = "DEF";
  var $v3 = false;
  var $v4 = 'z';
  var $v5 = [1,2,3,4];

  var $v6 = [
    "foobar" => 8,
    "barfoo" => 9,
  ];

  var $v7;

  function set(){
    $v7 = new t2();
  }
}

class t2{
  var $v8 = null;
}

function test(){
  $v = new t1();
  $v->set();
  echo wddx_serialize_value($v);
  print "\n";
}
test();
