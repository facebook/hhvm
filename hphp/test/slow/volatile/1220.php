<?php

function foo() {
  if (class_exists('bar')) {
    echo "yes\n";
  }
 else {
    echo "no\n";
  }
  class bar {
  }
  if (class_exists('bar')) {
    echo "yes\n";
  }
 else {
    echo "no\n";
  }
}
foo();
