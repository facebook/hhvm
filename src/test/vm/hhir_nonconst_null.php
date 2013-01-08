<?php

/*
 * This is a small test case for handling comparisons on null SSATmps
 * that aren't isConst().
 */
function ar() { return null; }
function foo() {
  $x = ar();
  echo $x != true;
  echo "\n";
  echo $x == true;
  echo "\n";
  echo $x != false;
  echo "\n";
  echo $x == false;
  echo "\n";
}
foo();
