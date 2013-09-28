<?php

function foo() {
  if (function_exists('bar')) {
    echo "yes\n";
  }
 else {
    echo "no\n";
  }
  function bar() {
    echo "bar\n";
  }
  if (function_exists('bar')) {
    echo "yes\n";
  }
 else {
    echo "no\n";
  }
}
foo();
