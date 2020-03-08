<?hh // strict

namespace NS_default_arguments;

function f1(int $p1 = 10, float $p2 = 1.23, bool $p3 = true, mixed $p4 = null, string $p5 = "abc",
  ?array<mixed> $p6 = [1,array()]): void {
  $argList = func_get_args();
  echo "f1: # arguments passed is ".count($argList)."\n";

  foreach ($argList as $k => $e) {
    echo "\targ[$k] = >$e<\n";
  }
  var_dump($p1, $p2, $p3, $p4, $p5, $p6);
}

/*
// if any leading parameter has a default argument then all those following must have one too.

// 2 default followed by one non-default; unusual, but permitted

function f2(int $p1 = 100, int $p2 = 1.23, int $p3): void {
}
*/

// 1 non-default followed by two default

function f4(int $p1, int $p2 = -100, int $p3 = -200): void {
  $argList = func_get_args();
  echo "f4: # arguments passed is ".count($argList)."\n";

  foreach ($argList as $k => $e) {
    echo "\targ[$k] = >$e<\n";
  }
  var_dump($p1, $p2, $p3);
}

function f3(int $p1 = -1, float $p2 = 99.99, string $p3 = '??'): void {
  echo "\$p1 is $p1, \$p2 is $p2, \$p3 is $p3\n";
}

function main(): void {
  f1();
  f1(12);
//  f1(10.5);	// mismatch; rejected
  f1(12, 10.5);
  f1(12, 10.5, false);
  f1(12, 10.5, false, 100);
  f1(12, 10.5, false, 'aaa', 'xxx');
  f1(12, 10.5, false, 'aaa', 'xxx', null);

//  f4();		// too few arguments
  f4(10);
  f4(10, 20);
  f4(10, 20, 30);

  f3();				// $p1 is -1, $p2 is 99.99, $p3 is ??
  f3(123);			// $p1 is 123, $p2 is 99.99, $p3 is ??
  f3(123, 3.14);		// $p1 is 123, $p2 is 3.14, $p3 is ??
  f3(123, 3.14, 'Hello');	// $p1 is 123, $p2 is 3.14, $p3 is Hello
}

/* HH_FIXME[1002] call to main in strict*/
main();
