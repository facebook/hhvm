<?php

class X {
  function __construct() {
 echo 'construct
';
 }
  function __destruct() {
 echo 'destruct
';
 }
}
function test() {
  $a = new X;
  echo 'before unset
';
  unset($a);
  echo 'after unset
';
}
test();
