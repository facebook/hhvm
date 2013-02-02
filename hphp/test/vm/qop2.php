<?php
function f() {
  $a = true;
  $b = true;
  echo ($a && $b) ? "yes\n" : "no\n";
  echo ($a || $b) ? "yes\n" : "no\n";
  echo "\n";
  $a = true;
  $b = false;
  echo ($a && $b) ? "yes\n" : "no\n";
  echo ($a || $b) ? "yes\n" : "no\n";
  echo "\n";
  $a = false;
  $b = true;
  echo ($a && $b) ? "yes\n" : "no\n";
  echo ($a || $b) ? "yes\n" : "no\n";
  echo "\n";
  $a = false;
  $b = false;
  echo ($a && $b) ? "yes\n" : "no\n";
  echo ($a || $b) ? "yes\n" : "no\n";
  echo "\n";
}
f();

