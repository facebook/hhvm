<?hh

function outputPrototype($rf) :mixed{
  try {
    $prototype = $rf->getPrototype();
    print $prototype->getDeclaringClass()->getName() .
      "::" . $prototype->getName();
    print "\n";
  } catch (ReflectionException $re) {
    print $re->getMessage(). "\n";
  }
}

interface Int1 {
  function method1():mixed;
}
interface Int2 {
  function method2():mixed;
}
interface Int3 extends Int2 {
  function method3():mixed;
}
interface Int4 {
  function method4():mixed;
}

class Cls1 {
  public function method1() :mixed{
  }
  private function method2() :mixed{
  }
}
class Cls2 {
  public function method1() :mixed{
  }
  public function method2() :mixed{
  }
}
abstract class Cls3 extends Cls2 implements Int3 {
}
class Cls4 extends Cls3 {
  public function method3() :mixed{
  }
}
class Cls5 extends Cls4 implements Int4 {
  public function method4() :mixed{
  }
}

trait Method3 {
  public function method3() :mixed{
  }
}

class Cls6 extends Cls4 implements Int4 {
  use Method3;
  function method4() :mixed{
  }
}
class Cls7 extends Cls6 {
  public function method1() :mixed{
  }
  public function method7() :mixed{
  }
}
class Cls8 extends Cls7 {
  public function method7() :mixed{
  }
}

class PDOSubClass extends PDO {
  public function commit() :mixed{
    return true;
  }
}

function main() :mixed{
  $tests = vec[
    vec["Cls1", "method1"],
    vec["Cls1", "method2"],
    vec["Cls2", "method2"],
    vec["Cls3", "method1"],
    vec["Cls3", "method3"],
    vec["Cls4", "method1"],
    vec["Cls4", "method3"],
    vec["Cls5", "method3"],
    vec["Cls5", "method4"],
    vec["Cls6", "method3"],
    vec["Cls6", "method4"],
    vec["Cls6", "method1"],
    vec["Cls7", "method1"],
    vec["Cls7", "method7"],
    vec["Cls8", "method1"],
    vec["Cls8", "method7"],
    // builtin class
    vec["PDO", "commit"],
    // sublcass of builtin class
    vec["PDOSubClass", "commit"],
  ];

  foreach ($tests as $test) {
    $rf = new ReflectionMethod($test[0], $test[1]);
    outputPrototype($rf);
  }
}



<<__EntryPoint>>
function main_reflection_method_get_prototype() :mixed{
main();
}
