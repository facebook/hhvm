<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

// Test behavior around exceptions leaking out of destructors.
// Specifically, fatals vs. user exceptions.

class Ex1 extends Exception {
}

class Ex2 extends Exception {
}

class ThrowDestruct1 {
  public function __destruct() {
    printf("In ThrowDestruct1::__destruct()\n");
    throw new Ex1('Exception leaked out of ThrowDestruct1::__destruct()');
  }
}

class ThrowDestruct2 {
  public function __destruct() {
    printf("In ThrowDestruct2::__destruct()\n");
    throw new Ex2('Exception leaked out of ThrowDestruct2::__destruct()');
  }
}

class ExitDestruct {
  public function __destruct() {
    printf("In ExitDestruct::__destruct()\n");
    exit();
  }
}

function bar() {
  $td2 = new ThrowDestruct2;

  // This exit() should prevent the destructor
  // for $td2 from running, as well as any up-stack destructors.
  exit();
}

function foo() {
  $td1 = new ThrowDestruct1;
  printf("Calling bar()\n");
  bar();
  printf("After bar()\n");
}

function printPreviousExceptions($e) {
  $i = 0;

  while ($e = $e->getPrevious()) {
    printf("\tPrevious #%d: %s\n", ++$i, $e->getMessage());
  }
}

function main() {
  printf("main() starting\n");

  try {
    printf("Calling foo()\n");
    foo();
    printf("After foo()\n");
  }
  catch (Exception $e) {
    printf("Caught %s in main()\n", $e->getMessage());
    printPreviousExceptions($e);
  }

  printf("main() ending\n");
}

printf("Calling main()\n");
main();
printf("Returned from main(), exiting\n");
