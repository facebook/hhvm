<?hh // strict

namespace NS_limits_on_types_in_arithmetic;

function doit(num $p1_num, ?int $p2_nint, ?float $p3_nfloat, ?num $p4_nnum): void {
//  var_dump(true + 1);		// can't do arithmetic on a bool
  var_dump(123 + 1);		// Okay
  var_dump(2.4 + 1);		// Okay
  var_dump($p1_num + 1);	// Okay
//  var_dump("123" + 1);	// can't do arithmetic on a string, not even a numeric string
//  var_dump("abc" + 1);	// can't do arithmetic on a string
//  var_dump(null + 1);		// can't do arithmetic on a nullable type

//  var_dump($p2_nint + 1);	// can't do arithmetic on a nullable type
//  var_dump($p3_nfloat + 1);	// can't do arithmetic on a nullable type
//  var_dump($p4_nnum + 1);	// can't do arithmetic on a nullable type
}

function main(): void {
  doit(10, 20, 3.45, null);
}

/* HH_FIXME[1002] call to main in strict*/
main();
