<?php

class X {
  function __toString() { return __METHOD__; }
}

function test($a) {
  var_dump($a[-1] = new X);
  var_dump($a);
}



<<__EntryPoint>>
function main_promote_string2() {
test("");
echo "1\n";
test("x");
echo "2\n";
}
