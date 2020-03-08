<?hh // strict

namespace NS_functions_recursion;

function main(): void {
  for ($i = 0; $i <= 10; ++$i) {
    printf("%2d! = %d\n", $i, factorial($i));
  }
}

// use recursion to implement a factorial function
// Note: can call a function prior to its definition

function factorial(int $int): int {
  return ($int > 1) ? $int * factorial($int - 1) : $int;
}

/* HH_FIXME[1002] call to main in strict*/
main();
