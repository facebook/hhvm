<?hh // strict

namespace NS_anonymous_functions;

class Cz {
  private ?(function (int): int) $pr1;
  private array<(function (int): int)> $pr2;
  private ?array<(function (int): int)> $pr3;
  private ?array<?(function (int): int)> $pr4;

/*
If the initialization of pr2 is omitted from the constructor below, get the following message:

The class member pr2 is not always properly initialized
Make sure you systematically set $this->pr2 when the method __construct is called
Alternatively, you can define the type as optional (?...)
*/
  public function __construct() {
    $this->pr2 = array(
      function (int $p): int { return $p * 2; }, 
      function (int $p): int { return $p * $p; }
    );
  }
}


interface I {}
class C implements I {}

function double(int $p): int {
  return $p * 2;
}

function square(int $p): int {
  return $p * $p;
}

function doit(int $value, (function (int): int) $process): int {
  var_dump($process);

  return $process($value);
}

function compute(array<int> $values): void {
  $count = 0;
        
//  $callback = function () use (&$count)
  $callback = function () use ($count) {
    echo "Inside method >>" . __METHOD__ . "<<\n";	// called {closure}
    ++$count;
  };

  $callback();
  echo "\$count = $count\n";
  $callback();
  echo "\$count = $count\n";
}

class D {
  private function f(): void {
    echo "Inside method >>" . __METHOD__ . "<<\n";
  }

  public function compute(array<string, int> $values): void {
    $count = 0;
        
//  $callback = function (int $p1, int $p2): void use (&$count, $values)
    $callback = function (int $p1, int $p2): void use ($count, $values) {
      echo "Inside method >>" . __METHOD__ . "<<\n";	// called D::{closure}
      ++$count;

      $this->f();	// $this is available automatically; can't put it in use clause anyway
    };

    echo "--\n";
    var_dump(gettype($callback));
    echo "--\n";
    var_dump($callback);
    echo "--\n";
    var_dump($callback instanceof \Closure);
    echo "--\n";

    $callback(1, 2);
    echo "\$count = $count\n";
    $callback(5, 6);
    echo "\$count = $count\n";
        
    $callback2 = function(): void {
      echo "Inside method >>" . __METHOD__ . "<<\n";	// ALSO called D::{closure}
    };

    echo "--\n";
    var_dump(gettype($callback2));
    echo "--\n";
    var_dump($callback2);
    echo "--\n";
    var_dump($callback2 instanceof \Closure);
    echo "--\n";

    $callback2();
  }

  public static function stcompute(array<string, int> $values): void {
    $count = 0;
        
//    $callback = function ($p1, $p2) use (&$count, $values)
    $callback = function (int $p1, int $p2): void use ($count, $values) {
      echo "Inside method >>" . __METHOD__ . "<<\n";	// called D::{closure}
      ++$count;
    };

    echo "--\n";
    var_dump(gettype($callback));
    echo "--\n";
    var_dump($callback);
    echo "--\n";
    var_dump($callback instanceof \Closure);
    echo "--\n";

    $callback(1, 2);
    echo "\$count = $count\n";
    $callback(5, 6);
    echo "\$count = $count\n";
  }
}

// (corrected) example from the Hack doc site

function foo_closure(string $adder_str): (function (string): int) {
  return function($to_str) use ($adder_str) {
    return strlen($to_str) + strlen($adder_str);
  };
}

function main_closure_example(): void {
  $hello = foo_closure("Hello");
  $facebook = foo_closure("Facebook");
  $fox = foo_closure("Fox");

  echo $hello("World") . "\n";
  echo $facebook("World") . "\n";
  echo $fox("World") . "\n";
}

// example from the Hack doc site
// Completely contrived

function f1((function(int, int): string) $x): string {
  return $x(2,3);
}

function f2(): string {
//  $c = function($n, int $m) {
//  $c = function($n, $m) {
  $c = function(int $n, int $m): string {
    $r = '';
    for ($i=0; $i<$n+$m; $i++) {
      $r .= "hi";
    }
    return $r;
  };
  return f1($c);
}

function main(): void {
  echo "----------------- main_closure_example from Hack doc site ----------------------\n";

  main_closure_example();

  echo "Calling f2 =>" . f2() . "<\n";

  echo "----------------- closure with no parameters ----------------------\n";

//  $cl1 = function ()
//  $cl1 = function (): int
  $cl1 = function (): void {
    echo "Inside function >>" . __FUNCTION__ . "<<\n";
    echo "Inside method >>" . __METHOD__ . "<<\n";
    // ...
//  return 123;	// test
  };

  echo "--\n";
  var_dump(gettype($cl1));
  echo "--\n";
  var_dump($cl1);
  echo "--\n";
  var_dump($cl1 instanceof \Closure);
  echo "--\n";

  $cl1();
//  $x = $cl1();		// test
//  echo "\$x = $x\n";	// test

  // Closure object is empty

  echo "----------------- closure with 5 parameters ----------------------\n";

//  $cl2 = function ($p1, $p2, $p3, $p4, $p5)
  $cl2 = function (int $p1, int $p2, array<int> $p3, C $p4, I $p5): void {
    echo "Inside function >>" . __FUNCTION__ . "<<\n";
    echo "Inside method >>" . __METHOD__ . "<<\n";
    // ...
  };

  echo "--\n";
  var_dump(gettype($cl2));
  echo "--\n";
  var_dump($cl2);
  echo "--\n";
  var_dump($cl2 instanceof \Closure);
  echo "--\n";

  $cl2(10, 20, array(1, 2), new C(), new C());

  echo "----------------- passing a callable to a function ----------------------\n";

  $res = doit(10, fun('\NS_anonymous_functions\double'));
  echo "Result of calling doit using function double = $res\n-------\n";

  $res = doit(10, fun('\NS_anonymous_functions\square'));
  echo "Result of calling doit using function square = $res\n-------\n";

  $doubleFun = function (int $p): int { return $p * 2; };
  $squareFun = function (int $p): int { return $p * $p; };
	
  $res = doit(5, $doubleFun);
  echo "Result of calling doit using double closure = $res\n-------\n";

  $res = doit(5, $squareFun);
  echo "Result of calling doit using square closure = $res\n-------\n";

  $functionList = array($doubleFun, $squareFun);
  echo "\$functionList[0] returns " . $functionList[0](12) . "\n";
  echo "\$functionList[1] returns " . $functionList[1](15) . "\n";
  var_dump($functionList);

  echo "----------------- using a use clause, #1 ----------------------\n";

  compute(array(1,2,3));

  echo "----------------- using a use clause, #2 (instance method) ----------------------\n";

  $d1 = new D();
  $d1->compute(array("red" => 3, "green" => 10));

  echo "----------------- using a use clause, #3 (static method) ----------------------\n";

  D::stcompute(array("red" => 3, "green" => 10));

  echo "----------------- Misc. Stuff ----------------------\n";

//  (function (): void { echo "Hi\n"; })();		// can't use an anon function directly with ()
  $v = (function (): void { echo "Hi\n"; });
  $v();

// can have attributes, default values, and ...

  $v = (function (<<XX>> int $p = 123, ...): void { echo "\$p = $p\n"; });
  $v(6, 7, 8);
  $v(-5);
  $v();
//  $v(10.5);	// float is incompatible with int

// parameter type can be omitted and implied from default value

  $v = (function ($p = 123): void { echo "\$p = $p\n"; });
  $v(-5);
  $v();
//  $v(10.5);	// float is incompatible with int

// parameter type can be omitted

  $v = (function ($p): void { echo "\$p = $p\n"; });
  $v(-5);
  $v(10.5);

// return type can be omitted and inferred

  $v = (function ($p) { echo "\$p = $p\n"; });
  $v(-5);
  $v(10.5);
}

/* HH_FIXME[1002] call to main in strict*/
main();
