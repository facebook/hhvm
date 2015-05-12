<?php
function foo() {
  try {
  } finally {
    goto test;
    test:
      echo "blah\n";
  }
}
foo();
echo "Done\n";
