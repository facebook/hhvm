<?php

function f() {
  $a = array();
  echo $a[15410];
  echo "In f: " . error_reporting() . "\n";
}


echo error_reporting() . "\n";
f();
echo error_reporting() . "\n";
@f();
echo error_reporting() . "\n";

function g() {
  $a = array();
  echo $a[15411];
  echo "In g: " . error_reporting() . "\n";
  error_reporting(15251);
}

echo error_reporting() . "\n";
@g();
echo error_reporting() . "\n";

$arr = array();
echo @$arr['nope'];

