<?php

class Ex1 extends Exception {
  function getString() {
    return "Ex1\n";
  }
}

class Ex2 extends Exception {
  function getString() {
    return "Ex2\n";
  }
}

function foo() {
  throw new Ex2();
}

try {
  foo();
  echo "should not be here\n";
} catch (Ex1 $e) {
  $a = $e->getString();
  echo "caught exception $a";
} catch (Ex2 $e) {
  $a = $e->getString();
  echo "caught exception $a";
}

echo "after exception\n";
