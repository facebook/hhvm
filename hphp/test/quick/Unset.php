<?php

function f() {
  $n = "y";
  $g = "z";
  global $$g;

  print ":".isset($x).":\n";
  print ":".isset($$n).":\n";
  print ":".isset($$g).":\n";

  unset($x);
  unset($$n);
  unset($$g);
  print ":".isset($x).":\n";
  print ":".isset($$n).":\n";
  print ":".isset($$g).":\n";

  $x = 0;
  $$n = 0;
  $$g = 0;
  print ":".isset($x).":\n";
  print ":".isset($$n).":\n";
  print ":".isset($$g).":\n";

  unset($x);
  unset($$n);
  unset($$g);
  print ":".isset($x).":\n";
  print ":".isset($$n).":\n";
  print ":".isset($$g).":\n";
}
f();

class A {
  public function foo() {
    unset($this);
  }
}
$obj = new A;
$obj->foo();
