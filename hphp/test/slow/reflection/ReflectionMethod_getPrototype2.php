<?hh
// Test prototype of builtin classes

function outputPrototype(ReflectionMethod $rf) {
  try {
    $prototype = $rf->getPrototype();
    print $prototype->getDeclaringClass()->getName() .
      "::" . $prototype->getName();
    print "\n";
  } catch (ReflectionException $re) {
    print $re->getMessage(). "\n";
  }
}

class SimpleXMLElementChild extends SimpleXMLElement {
}


function main() {
  $tests = array(
    tuple(WaitHandle::class, "getWaitHandle"),
    tuple(WaitHandle::class, "join"),
    tuple(WaitableWaitHandle::class, "getWaitHandle"),
    tuple(WaitableWaitHandle::class, "join"),
    tuple(Generator::class, "next"),
    tuple(Generator::class, "getOrigFuncName"),
    tuple(SimpleXMLElementChild::class, '__construct'),
    tuple(SimpleXMLElement::class, 'offsetSet'),
    tuple(SimpleXMLElementChild::class, 'offsetSet'),
  );

  foreach ($tests as $test) {
    outputPrototype(new ReflectionMethod(...$test));
  }
}

main();

