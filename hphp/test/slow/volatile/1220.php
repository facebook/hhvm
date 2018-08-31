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

<<__EntryPoint>>
function main_1220() {
foo();
}
