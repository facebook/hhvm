<?php

function b($s) {
  if (strlen($s) == 4) {
    return 12;
  }
  $z = 234;
  return 8;
}

function a($b) {
  if ($b) {
    $x = 'floo';
  } else {
    $x = 'fleeee';
  }
  b($x);
}

function f() {
  for ($i = 0; $i < 4; ++$i) {
    a($i < 3);
    $tmp = $i;
    $i = $tmp;
  }
}

function enable() {
  echo "Going to enable\n";
  fb_enable_code_coverage();
  echo "Enabled\n";
  $z = 3;
}

function doenable() {
  $y = 2;
  enable();
  $y += 43;
}

function main() {
  echo "About to enable\n";
  doenable();
  echo "Done enabling\n";
  f();
  $r = fb_disable_code_coverage();
  var_dump($r);
}
main();
