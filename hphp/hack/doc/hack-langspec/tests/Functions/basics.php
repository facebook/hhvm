<?hh // strict

namespace NS_functions_basics;

// Function names are not case-sensitive

function f(): void { echo "f\n"; }
//function F(): void { echo "F\n"; }	// F is equivalent to f

// function having no declared parameters

function f1(...): void {
  $argList = func_get_args();
  echo "f1: # arguments passed is ".count($argList)."\n";

  foreach ($argList as $k => $e) {
    echo "\targ[$k] = >$e<\n";
  }
}

// function having 2 declared parameters

function f2(int $p1, int $p2): void {
  // A null value doesn't prove the argument wasn't passed; find a better test

  echo "f2: \$p1 = ".($p1 == null ? "null" : $p1).
    ", \$p2 = ".($p2 == null ? "null" : $p2)."\n"; 
}

function square(num $v): num {
  return $v * $v;
}

function addVector(Vector<int> $v1, Vector<int> $v2): Vector<int> {
  $result = Vector{};

  // ...

  return $result;
}

function main(): void {
//  var_dump(f1());	// call f1, default return value is null
//  $f = f1;	// PHP: assign this string to a variable; Hack disallows
//  $f();		// PHP: call f1 indirectly via $f; Hack disallows

  f1();
  f1(10);
  f1(true, "green");
  f1(23.45, null, array(1,2,3));

  f2(10, 20);

// some simple examples of function calls

  echo "5 squared = " . square(5) . "\n";
  echo strlen("abcedfg")."\n";

  $v = addVector(Vector {10, 20}, Vector {60, 30, 10});
  var_dump($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();
