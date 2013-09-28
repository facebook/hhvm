<?php

class X {
  function __destruct() { var_dump(__METHOD__); }
  function __toString() { return __METHOD__; }
}

function test($a) {
  var_dump($a[-1] = new X);
  var_dump($a);
}

test("");
echo "1\n";
test("x");
echo "2\n";

