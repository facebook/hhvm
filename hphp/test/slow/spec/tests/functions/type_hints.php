<?hh

function f1(varray $p1)
:mixed{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

class C1 {}
class D1 extends C1 {}

function f2(C1 $p1)
:mixed{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

function f3(object $p1)
:mixed{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

interface I1 {}
interface I2 extends I1 {}
class C2 implements I1 {}
class D2 extends C2 implements I2 {}

function f4(I1 $p1)
:mixed{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}
<<__DynamicallyCallable>>
function hello()
:mixed{
    echo "Hello!\n";
}

function f5(callable $p1)
:mixed{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
    $p1();
}

function f6(inout C1 $p1)
:mixed{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  echo "--------------- test type hint array ---------------------\n";

  // f1();    // Argument 1 passed to f1() must be of the type array, none given
  // f1(123); // Argument 1 passed to f1() must be of the type array, integer given
  f1(vec[10,20]);

  echo "--------------- test type hint class-name ---------------------\n";

  //f2(123); // Argument 1 passed to f1() must be an instance of C1, integer give
  //f2([10,20]);    // Argument 1 passed to f2() must be an instance of C1, array given
  f2(new C1);
  f2(new D1);

  echo "--------------- test type hint object ---------------------\n";

  //f3(123); // Argument 1 passed to f1() must be an instance of object, integer given
  //f3([10,20]);    // Argument 1 passed to f2() must be an instance of object, array given
  //f3(new C1);         // must be an instance of object, instance of C1 given

  // object is not a special/recognized marker in this context

  echo "--------------- test type hint interface-name ---------------------\n";

  //f4(123); // must implement interface I1, integer given
  f4(new C2);
  f4(new D2);

  echo "--------------- test type hint callable ---------------------\n";

  //f5(123); // must be callable, integer given
  f5('hello');

  echo "--------------- test type hint + by inout ---------------------\n";
  $x = new C1;
  f6(inout $x);
}
