<?hh
// Test prototype of builtin classes

function outputPrototype(ReflectionMethod $rf) :mixed{
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


function main() :mixed{
  $tests = vec[
    tuple(Generator::class, "next"),
    tuple(Generator::class, "getOrigFuncName"),
    tuple(SimpleXMLElementChild::class, '__construct'),
    tuple(SimpleXMLElement::class, 'offsetSet'),
    tuple(SimpleXMLElementChild::class, 'offsetSet'),
  ];

  foreach ($tests as $test) {
    outputPrototype(new ReflectionMethod(...$test));
  }
}



<<__EntryPoint>>
function main_reflection_method_get_prototype2() :mixed{
main();
}
