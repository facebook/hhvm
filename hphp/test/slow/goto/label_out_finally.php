<?php
function foo($a) {
  try {
    if ($a < 5) {
      echo "Foo";
    }
    else {
      test:
        echo "bar";
    }
  } finally {
    goto test;
  }
}
foo(5);
echo "Done\n";
