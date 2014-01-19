<?php

function f() {
  $n = "x";
  $g = "y";
  global $$g;

  $x = 0;
  $$g = 0;
  print ":".empty($x).":\n";
  print ":".empty($$n).":\n";
  print ":".empty($$g).":\n";

  $x = 1;
  $$g = 1;
  print ":".empty($x).":\n";
  print ":".empty($$n).":\n";
  print ":".empty($$g).":\n";
}
f();
