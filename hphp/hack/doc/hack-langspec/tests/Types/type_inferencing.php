<?hh // strict

namespace NS_type_inferencing;

class C {
  const C1 = 10;	// type omitted; int inferred from initializer
  const string C2 = "red";
}

function f(int $p1 = -1): void {
			// on entry to the function, $p1 has the declared type int
  var_dump($p1);
  $p1 = 23.56;		// $p1 has the inferred type float
  var_dump($p1);

  $v = 'acb';		// $v has type string
  var_dump($v);
  $v = true;		// $v has type bool
  var_dump($v);
  $v = array('red' => 10, 'green' => 15, 'white' => 20); // $v has type map-like array of int
  var_dump($v);
  $v = new C();		// $v has type C
  var_dump($v);
}

function main (): void {
  f();
}

/* HH_FIXME[1002] call to main in strict*/
main();
