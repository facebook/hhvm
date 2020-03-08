<?hh // strict

namespace NS_null_safe_calls;

class D {}

class C {
  public function f1(int $m, int $n): D {
    echo "Inside " . __METHOD__ . "\n";
    return new D();
  }

  public function f2(int $m, int $n): ?D {
    echo "Inside " . __METHOD__ . "\n";
    return null;
  }

  public function f3(int $m, int $n): void {
    echo "Inside " . __METHOD__ . "\n";
  }
}

/*
Removing the ? from the return type, as in

function test(?C $x): D

results in

Invalid return type in return statement
  Return statement: This is an object of type NS_null_safe_method_call\D
  It is incompatible with a nullable type, [as inferred by the ?-> operator]
*/

function hello(int $j): int {
  echo "Inside " . __METHOD__ . "; \$j = $j\n";
  return $j;
}

function test(?C $x): void {
  $i = 5;

  echo "------------ \$x?->f1(++\$i, hello(\$i))\n";

  $res1 = $x?->f1(++$i, hello($i));
  var_dump($res1);

  echo "------------ \$x?->f2(++\$i, hello(\$i))\n";

  $res2 = $x?->f2(++$i, hello($i));
  var_dump($res2);

  echo "------------ \$x?->f3(++\$i, hello(\$i))\n";

  $res3 = $x?->f3(++$i, hello($i));	// allows >-> to call a void function thus producing a ?void result
  var_dump($res3);
}

function main(): void {
  echo "------------ test(new C())\n";

  test(new C());

  echo "------------ test(null)\n";

  test(null);	// the expressions in the method argument list DO GET CALLED ANYWAY
}

/* HH_FIXME[1002] call to main in strict*/
main();
