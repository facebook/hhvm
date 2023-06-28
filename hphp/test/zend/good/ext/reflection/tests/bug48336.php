<?hh
class A {
}

class B extends A {
  static protected $prop;
}

class C extends B {
  static protected $prop;
}

class D extends C {
}

class E extends D {
}

class F extends E {
  static protected $prop;
}
<<__EntryPoint>>
function main() :mixed{
  for($i = ord('A'); $i <= ord('F'); $i++) {
    $class = chr($i);
    print($class.' => ');
    try {
      $rp = new ReflectionProperty($class, 'prop');
      print($rp->getDeclaringClass()->getName());
    } catch(Exception $e) {
      print('N/A');
    }
    print("\n");
  }
}
