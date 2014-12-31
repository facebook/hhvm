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

class BlockableWaitHandleSubClass extends BlockableWaitHandle {
  public function getWaitHandle() {
  }
  public function getContextIdx() {
  }
}

function main() {
  $tests = array(
    array(WaitHandle::class, "getWaitHandle"),
    array("Generator", "next"),
    array(BlockableWaitHandle::class, "getContextIdx"),
    array("BlockableWaitHandleSubClass", "getWaitHandle"),
    array("BlockableWaitHandleSubClass", "getContextIdx"),
  );

  foreach ($tests as $test) {
    $rf = new ReflectionMethod($test[0], $test[1]);
    outputPrototype($rf);
  }
}

main();

