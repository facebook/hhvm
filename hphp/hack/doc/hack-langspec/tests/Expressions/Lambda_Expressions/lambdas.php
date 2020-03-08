<?hh // strict

namespace NS_lambdas;

function concatStr(): (function (string): string) {
  $x = 'XX:';
  return function (string $y): string use ($x) {
    return $x . $y;
  };
}

function lam1(int $b): int {
// $b is captured from the parameter in this function body

// ==> has the same precedence as =, and associates right-to-left

// function body is an expression

//  $fn = $a ==> $a + $b;
//  $fn = $a ==> ($a + $b);
//  $fn = ($a ==> ($a + $b));

// function body is a compound statement ending with a return statement whose value becomes resulting expression
// compound statement can be empty, which is just like a non-empty one not having a return; implicit void return

  $fn = $a ==> { return $a + $b; };

  return $fn(10);
}

function main(): void {
  echo "-------------- anonymous function concatStr ----------------\n\n";

  $fn = concatStr();
  echo $fn('blue') . "\n";
  echo $fn('green') . "\n";

  echo "\n-------------- lam1 ----------------\n\n";

  var_dump(lam1(42)); // return 52

  echo "\n-------------- misc ----------------\n\n";

  $fn = $a ==> $a << 3;
  var_dump($fn(10));
//  var_dump($fn(5.6));	// float is incompatible with <<

  echo "\n-------------- empty compound statement ----------------\n\n";

  $fn = $a ==> {};
//  var_dump($fn(123));		// can't use the resturn value form a void function

  echo "\n-------------- single parameter, type inferred ----------------\n\n";

  $fn = $a ==> $a * 2;
  var_dump($fn(10));
  var_dump($fn(5.6));

  echo "\n-------------- single parameter in parens, type inferred ----------------\n\n";

  $fn = ($a) ==> $a * 2;
  var_dump($fn(10));
  var_dump($fn(5.6));

/*
  echo "\n-------------- single parameter in parens with attribute, type inferred ----------------\n\n";

  $fn = (<<XX>> $a) ==> $a * 2;	// hhvm gags on attribute; reports "unexpected T_SL"
  var_dump($fn(10));
  var_dump($fn(5.6));
*/
  echo "\n-------------- single parameter in parens, but with ...; type inferred ----------------\n\n";

  $fn = ($a, ...) ==> $a * 2;
  var_dump($fn(10));
  var_dump($fn(5.6));
  var_dump($fn(3, 4, 5));	// okay; 2nd and 3rd args are ignored

  echo "\n-------------- single parameter in parens, type set by default value ----------------\n\n";

  $fn = ($a = -90) ==> $a * 2;
  var_dump($fn(10));
//  var_dump($fn(5.6));	// float is incompatible with int
  var_dump($fn());

  echo "\n-------------- single parameter in parens, explicit type ----------------\n\n";

// need parens of have explicit type

  $fn = (int $a) ==> $a * 2;
  var_dump($fn(10));
//  var_dump($fn(5.6));	// float is incompatible with int

  echo "\n-------------- no parameters, but parens necessary ----------------\n\n";

  $fn = () ==> 100;
  var_dump($fn());

  echo "\n-------------- multiple parameters, types inferred ----------------\n\n";

  $fn = ($a, $b, $c) ==> $a + $b + $c;
  var_dump($fn(10, 5, 9));
  var_dump($fn(10.0, 5.0, 9.0));

  echo "\n-------------- multiple parameters, types partially/fully specified ----------------\n\n";

  $fn = (int $a, $b, $c) ==> $a + $b + $c;
  var_dump($fn(10, 5, 9));
//  var_dump($fn(10.0, 5.0, 9.0));	// float is incompatible with int

  $fn = (int $a, int $b, $c) ==> $a + $b + $c;
  var_dump($fn(10, 5, 9));

  $fn = (int $a, int $b, int $c) ==> $a + $b + $c;
  var_dump($fn(10, 5, 9));

  $fn = ($a, $b, int $c) ==> $a + $b + $c;
  var_dump($fn(10, 5, 9));

  echo "\n-------------- lambdas can be async ----------------\n\n";

  $fn = async $a ==> $a * 2;
  var_dump($fn(10));
  var_dump($fn(5.6));

  echo "\n-------------- specifying a return type ----------------\n\n";

  $fn = ($a): int ==> $a * 2;
  var_dump($fn(10));
//  var_dump($fn(5.6));	// float is incompatible with int (type of $a * 2)

  $fn = ($a): float ==> $a * 2.0;
  var_dump($fn(10));
  var_dump($fn(5.6));

  echo "\n-------------- chaining lambdas ----------------\n\n";

  $fn1 = $x ==> $y ==> $x + $y;
  $fn2 = $fn1(10);
  $res = $fn2(7);		// result is 17
  echo "Result is " . $res . "\n";

  echo "\n-------------- comparing brief lambdas with equivalent anonymous functions ----------------\n\n";

  $doublerl = ($p) ==> $p * 2;
  var_dump($doublerl);
  var_dump($doublerl(10));
  var_dump($doublerl(5.6));

  $doubler2 = function ($p) { return $p * 2; };
  var_dump($doubler2);
  var_dump($doubler2(10));
  var_dump($doubler2(5.6));
}

/* HH_FIXME[1002] call to main in strict*/
main();
