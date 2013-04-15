<?php

function main() {
  if (true) {
    function foo() {
      print("foo a\n");
    }
  } else {
    function foo() {
      print("foo b\n");
    }
  }

  foo();
}
main();
