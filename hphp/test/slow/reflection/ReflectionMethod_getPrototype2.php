<?php
// Test prototype of builtin classes

function outputPrototype($rf) {
  try {
    $prototype = $rf->getPrototype();
    print $prototype->getDeclaringClass()->getName() .
      "::" . $prototype->getName();
    print "\n";
  } catch (ReflectionException $re) {
    print $re->getMessage(). "\n";
  }
}

class SessionScopedWaitHandleSubClass extends SessionScopedWaitHandle {
  public function getWaitHandle() {
  }
  public function getContextIdx() {
  }
}

function main() {
  $tests = array(
    array("WaitHandle", "getWaitHandle"),
    array("Continuation", "next"),
    array("SessionScopedWaitHandle", "getContextIdx"),
    array("SessionScopedWaitHandleSubClass", "getWaitHandle"),
    array("SessionScopedWaitHandleSubClass", "getContextIdx"),
  );

  foreach ($tests as $test) {
    $rf = new ReflectionMethod($test[0], $test[1]);
    outputPrototype($rf);
  }
}

main();

