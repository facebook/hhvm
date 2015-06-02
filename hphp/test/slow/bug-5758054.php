<?php

// Check that extra function arguments aren't leaked when variable variables are
// used in the same function. See task #5758054.

class leaker {
  function __construct() {}
  function __destruct() { echo "leaker::__destruct\n"; }
}

function y() {
  $x = 'foo';
  $$x = 5;
}

y(new leaker());

echo "done\n";
