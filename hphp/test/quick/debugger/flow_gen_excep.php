<?php

// Test stepping with exceptions in continuations. This is a very simple test
// which simply confirms that exceptions flowing around continuations don't
// disturb step over.
function bar($a) {
  return $a + 2;
}

function genFoo($a) {
  $a = bar($a);
  try {
    $z = yield $a+5;
    $z++;
  } catch (Exception $e) {
    printf("Caught %s in genFoo()\n", $e->getMessage());
  }
  yield $a+1;
  error_log('Finished in genFoo');
}

function foo($a) {
  $gen1 = genFoo($a);
  $gen1->next();
  while ($gen1->valid()) {
    $val = $gen1->current();
    var_dump($val);
    try {
      $gen1->raise(new Exception('Exception given to continuation '.$val));
    } catch (Exception $e) {
      printf("Caught %s in foo()\n", $e->getMessage());
    }
  }
}

function main($a) {
  foo($a);
}

main(1);

