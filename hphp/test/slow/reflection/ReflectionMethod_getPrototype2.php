<?hh
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

class WaitableWaitHandleSubClass extends WaitableWaitHandle {
  public function getWaitHandle() {
  }
  public function getContextIdx() {
  }
}

function main() {
  $tests = array(
    array(WaitHandle::class, "getWaitHandle"),
    array("Generator", "next"),
    array(WaitableWaitHandle::class, "getContextIdx"),
    array("WaitableWaitHandleSubClass", "getWaitHandle"),
    array("WaitableWaitHandleSubClass", "getContextIdx"),
  );

  foreach ($tests as $test) {
    $rf = new ReflectionMethod($test[0], $test[1]);
    outputPrototype($rf);
  }
}

main();

