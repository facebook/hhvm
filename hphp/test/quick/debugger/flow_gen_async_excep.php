<?hh

// Test stepping with continuations and exceptions around throws. Specifically,
// we want to ensure that an exception handler executing at the same stack depth
// in one continuation isn't seen as flow control for a step operation in a
// different continuation at that depth. In this test, genThrow*Catch() run at
// the same stack depth as genFoo().

async function genThrowAndCatch($a) {
  var_dump($a);
  try {
    throw new Exception('Thrown from genThrowAndCatch '.$a);
  } catch (Exception $e) {
    printf("Caught %s in genThrowAndCatch()\n", $e->getMessage());
  }
  printf("Finished in genThrowAndCatch\n");
  return $a + 2;
}

async function genThrowNoCatch($a) {
  var_dump($a);
  throw new Exception('Thrown from genThrowAndCatch '.$a);
  return $a + 2;
}

async function genFoo($a) {
  var_dump($a);
  $a++;
  $a = await genThrowAndCatch($a);
  var_dump($a);
  try {
    $a = await genThrowNoCatch($a);
    var_dump($a);
  } catch (Exception $e) {
    printf("Caught %s in genFoo()\n", $e->getMessage());
  }
  printf("Finished in genFoo\n");
  return $a;
}

function foo($a) {
  $result = genFoo($a)->join();
  var_dump($result);
}

function main($a) {
  foo($a);
}

main(1);
