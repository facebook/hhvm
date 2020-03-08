<?hh // strict

namespace NS_pipe;

function fint(int $p): int {
  echo "Inside " . __FUNCTION__ . "; returning " . ($p + 3) . "\n";
  return $p + 3;
}

function gint(int $p): int {
  echo "Inside " . __FUNCTION__ . "; returning " . ($p * 2) . "\n";
  return $p * 2;
}

function hint(int $p): int {
  echo "Inside " . __FUNCTION__ . "; returning " . (-$p) . "\n";
  return -$p;
}

function fvoid(): void {
  echo "Inside " . __FUNCTION__ . "; returning nothing\n";
}

class Widget {
  public function getNumber(): int { return -1; }
}

function pipe_operator_example(array<Widget> $arr): int {
  return $arr
    |> array_map($x ==> $x->getNumber(), $$)
    |> array_filter($$, $x ==> $x % 2 == 0)
    |> count($$);
}

function main(): void {

// check where $$ is allowed (only in the RHS operand of |>)

//  $$ |> $$;	// Undefined variable: $$ (on LHS)

  20 |> $$;	// OK; on RHS of |>

//  $x = $$;	// Undefined variable: $$

//  $$ = 10;	// Undefined variable: $$; Invalid lvalue 

  echo "\n\n========================= Simple Use =========================\n\n";

  $v = (20 |> $$ |> $$);
  var_dump($v);

  20 |> var_dump($$);
  1.23 |> var_dump($$);
  "abc" |> var_dump($$);

//  20 |> $x = $$;	// Error: Feature not implemented: Assignment within pipe expressions
//  20 |> ($x = $$);	// Error: Feature not implemented: Assignment within pipe expressions

// The RH operand must use the result of the LH operand

//  echo "Result is: " . (fint(4) |> gint(9)) . "\n";	// Error: This expression does not contain a usage of the
						// special pipe variable. Did you forget to use the ($$)
						// variable? (Naming[2069]

//  fvoid() |> gint($$);	// You are using the return value of a void function
//  fvoid() |> 10 + $$;		// This is a num ... It is incompatible with void
  fvoid() |> $$ |> $$;		// Permitted because the result (which has type void) is not used

  echo "\n\n========================= Check Associativity =========================\n\n";

// associativity is left-to-right

  echo "Result is: " . (fint(4) |> gint($$) |> hint($$)) . "\n";
  echo "Result is: " . (fint(-5) |> gint($$) |> hint($$)) . "\n";

// precedence

// lower than additive operator ., as need grouping parens above

  echo "\n\n========================= Check Precedence against << =========================\n\n";

  2 << fint(1) |> gint($$);	// lower than <<, as g is passed (2 << fint(1))

/*
  2 <  fint(1) |> gint($$);	// lower than <, as g is passed a bool
  2 == fint(1) |> gint($$);	// lower than ==, as g is passed a bool
*/

  echo "\n\n========================= Check Precedence against &, |, and ^ =========================\n\n";

  7 & fint(1) |> gint($$);	// lower than &, as g is passed (7 & fint(1))
  7 | fint(1) |> gint($$);	// lower than |, as g is passed (7 | fint(1))
  7 ^ fint(1) |> gint($$);	// lower than ^, as g is passed (7 ^ fint(1))

/*
  1 && fint(1) |> gint($$);	// lower than &, as g is passed a bool
  0 && fint(1) |> gint($$);	// lower than &, as g is passed a bool
*/

  echo "\n\n========================= Check Precedence against ?: =========================\n\n";

  0 ? fint(1) |> gint($$) : fint(2) |> gint($$);
  1 ? fint(1) |> gint($$) : fint(2) |> gint($$);	// 3rd operand of ?: does NOT include gint($$)
  1 ? fint(1) |> gint($$) : (fint(2) |> gint($$));

  echo "\n\n========================= Check Precedence against ?? =========================\n\n";

  fint(1) |> gint($$) ?? fint(2) |> gint($$);	// lower than ??
  fint(1) |> gint($$) ?? (fint(2) |> gint($$));

  echo "\n\n========================= Check Precedence against a lambda =========================\n\n";

  $doit1 = ($p) ==> fint($p) |> gint($$);	// higher than ==>
  var_dump($doit1(2));
  $doit2 = ($p) ==> (fint($p) |> gint($$));	// parens are redundant
  var_dump($doit2(2));

  echo "\n\n========================= Check Precedence against = =========================\n\n";

  $x = fint(2) |> gint($$);	// higher than =
  $x = (fint(2) |> gint($$));	// parens are redundant
}

/* HH_FIXME[1002] call to main in strict*/
main();
