<?php

function a() { return 'a'; }
function b() { return 'b'; }

function foo() {
  $a = a();
  $b = b();
  return $a . $b;
}

function heh() {
  var_dump(foo());
}

heh();
