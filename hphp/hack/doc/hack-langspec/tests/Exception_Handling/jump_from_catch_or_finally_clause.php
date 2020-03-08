<?hh // strict

namespace NS_jump_from_catch_or_finally_clause;

function msg(): int {
  echo "In msg\n";
  return 999;
}

function f(): int {
  for ($i = 1; $i < 3; ++$i) {
    try {
      throw new \Exception();
    }
///*
    catch (\Exception $e) {
      echo "In handler for Exception\n";
//	break;			// allowed
//	continue;		// allowed
//	return msg();		// expression is evaluated, but value actually returned
				// when both returns exist is 20, from finally block, 
    }
//*/
    finally {
      echo "In finally block\n";
//	break;			// not allowed
//	continue;		// not allowed
//	return 20;		// not allowed
    }
  }
  return 1;
}

function main(): void {
  $r = f();
  var_dump($r);
}

/* HH_FIXME[1002] call to main in strict*/
main();
