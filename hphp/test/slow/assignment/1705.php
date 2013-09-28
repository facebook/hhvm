<?php

class X{
  function __destruct() {
 var_dump('destruct');
 }
}
function foo() {
  $x = new X;
  var_dump('before');
  $x = null;
  var_dump('after');
}
foo();
