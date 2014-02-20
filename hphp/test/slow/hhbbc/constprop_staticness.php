<?php

// This should ideally test with -fno-constant-prop, but we don't have
// a way to send options through to the test runner yet.

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
