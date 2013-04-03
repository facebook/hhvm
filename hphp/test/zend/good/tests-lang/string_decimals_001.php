<?php
function test($str) {
  echo "\n--> Testing $str:\n";
  var_dump((int)$str);
  var_dump((float)$str);
  var_dump($str > 0);
}

test("..9");
test(".9.");
test("9..");
test("9.9.");
test("9.9.9");
?>
===DONE===