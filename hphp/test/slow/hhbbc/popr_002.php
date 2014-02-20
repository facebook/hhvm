<?php

if (mt_rand()) { function& foo() { $x = 2; return $x; } }
else { function foo() { return 2; } }

function main() {
  foo();
}

main();
