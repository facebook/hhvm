<?php

function g() {
  exit;
  $x = exit;
  exit(0);
  $x = exit(0);
  exit + 3;
  $x = exit + 3;
  exit(1) + 3;
  $x = exit(1) + 3;
  f($x, exit($x), $x + 1);
}

call_user_func("g");
