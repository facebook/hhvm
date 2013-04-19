<?php

function b($s) {
  if (strlen($s) == 4) {
    return 12;
  }
  $GLOBALS['z'] = 234;
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
    $GLOBALS['tmp'] = $i;
    $i = $GLOBALS['tmp'];
  }
}

function enable() {
  echo "Going to enable\n";
  fb_enable_code_coverage();
  echo "Enabled\n";
  $GLOBALS['z'] = 3;
}

function doenable() {
  global $y;
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
