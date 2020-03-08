<?hh // strict

namespace NS_shapes2;

type S1 = shape('a' => int, 'b' => int);
type S2 = shape('b' => int, 'a' => int);

type S3 = shape('a' => int, 'b' => string);
type S4 = shape('b' => string, 'a' => int);

class C {
  public static S1 $p1 = shape('a' => 10, 'b' => 20);
  public static S2 $p2 = shape('b' => 20, 'a' => 10);

  public static S3 $p3 = shape('a' => 10, 'b' => 'bbb');
  public static S4 $p4 = shape('b' => 'bbb', 'a' => 10);

  public static shape('a' => int, 'n' => ?string) $p5a = shape('a' => 10, 'n' => null);
  public static shape('a' => int, 'n' => ?string) $p5b = shape('a' => 10);

  public shape('a' => int, 'n' => ?string) $p6a = shape('a' => 10, 'n' => null);
  public shape('a' => int, 'n' => ?string) $p6b = shape('a' => 10);

  public static (int, string) $p7 = tuple(10, 'x');
  public static (string, int) $p8 = tuple('x', 10);
}

function main(): void {
  f1(C::$p1);
  f2(C::$p2);

  echo "\nInside function " . __FUNCTION__ . "\n";

  echo (C::$p1 == C::$p2) ? "== is True\n" : "== is False\n";
  echo (C::$p1 != C::$p2) ? "!= is True\n" : "!= is False\n";
  echo (C::$p1 === C::$p2) ? "=== is True\n" : "=== is False\n";
  echo (C::$p1 !== C::$p2) ? "!== is True\n\n" : "!== is False\n\n";

  C::$p1 = C::$p2;
  echo (C::$p1 == C::$p2) ? "== is True\n" : "== is False\n";
  echo (C::$p1 != C::$p2) ? "!= is True\n" : "!= is False\n";
  echo (C::$p1 === C::$p2) ? "=== is True\n" : "=== is False\n";
  echo (C::$p1 !== C::$p2) ? "!== is True\n\n" : "!== is False\n\n";

  C::$p3 = C::$p4;
  echo (C::$p3 == C::$p4) ? "== is True\n" : "== is False\n";
  echo (C::$p3 != C::$p4) ? "!= is True\n" : "!= is False\n";
  echo (C::$p3 === C::$p4) ? "=== is True\n" : "=== is False\n";
  echo (C::$p3 !== C::$p4) ? "!== is True\n\n" : "!== is False\n\n";

  echo "\n=== null fields ===\n\n";

  $c = new C();
  var_dump(C::$p5a, C::$p5b, $c);

  echo "\n\n=== null field access ===\n\n";

  echo "\$p5a['a']: " . C::$p5a['a'] . "\n";
  echo "\$p5a['n']: " . C::$p5a['n'] . "\n";
  echo "\$p5b['a']: " . C::$p5b['a'] . "\n";
  echo "\$p5b['n']: " . C::$p5b['n'] . "\n";		// Undefined index: n
  echo "\$p6a['a']: " . $c->p6a['a'] . "\n";
  echo "\$p6a['n']: " . $c->p6a['n'] . "\n";
  echo "\$p6b['a']: " . $c->p6b['a'] . "\n";
  echo "\$p6b['n']: " . $c->p6b['n'] . "\n";		// Undefined index: n

  f3(shape('a' => 10, 'n' => 'xyz'));
  f3(shape('a' => 10, 'n' => null));
  f3(shape('a' => 10));					// Undefined index: n

/*
// as tuples are implemented as arrays, do they have issues re assignment and equality?

  echo "\n=== tuples ===\n";

  var_dump(C::$p7, C::$p8);
  echo (C::$p7 == C::$p8) ? "== is True\n" : "== is False\n";
  echo (C::$p7 != C::$p8) ? "!= is True\n" : "!= is False\n";
  echo (C::$p7 === C::$p8) ? "=== is True\n" : "=== is False\n";
  echo (C::$p7 !== C::$p8) ? "!== is True\n\n" : "!== is False\n\n";

  C::$p7 = C::$p8;
  echo (C::$p7 == C::$p8) ? "== is True\n" : "== is False\n";
  echo (C::$p7 != C::$p8) ? "!= is True\n" : "!= is False\n";
  echo (C::$p7 === C::$p8) ? "=== is True\n" : "=== is False\n";
  echo (C::$p7 !== C::$p8) ? "!== is True\n" : "!== is False\n";
*/
}

function f1(S2 $p): void {
  echo "\nInside function " . __FUNCTION__ . "\n";
  echo $p['a'] . ", " . $p['b'] . "\n";

  echo (C::$p1 == $p) ? "== is True\n" : "== is False\n";
  echo (C::$p1 != $p) ? "!= is True\n" : "!= is False\n";
  echo (C::$p1 === $p) ? "=== is True\n" : "=== is False\n";
  echo (C::$p1 !== $p) ? "!== is True\n\n" : "!== is False\n\n";
}

function f2(S1 $p): void {
  echo "\nInside function " . __FUNCTION__ . "\n";
  echo $p['a'] . ", " . $p['b'] . "\n";

  echo (C::$p2 == $p) ? "== is True\n" : "== is False\n";
  echo (C::$p2 != $p) ? "!= is True\n" : "!= is False\n";
  echo (C::$p2 === $p) ? "=== is True\n" : "=== is False\n";
  echo (C::$p2 !== $p) ? "!== is True\n\n" : "!== is False\n\n";
}

function f3(shape('a' => int, 'n' => ?string) $p): void {
  echo "\nInside function " . __FUNCTION__ . "\n";

  echo "\$p['a']: " . $p['a'] . "\n";
  echo "\$p['n']: " . $p['n'] . "\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
