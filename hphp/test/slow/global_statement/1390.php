<?php

$a = 0;
function test() {
  $a = 1;
  if (true) global $a;
  $a = 2;
}
test();
print "$a\n";
