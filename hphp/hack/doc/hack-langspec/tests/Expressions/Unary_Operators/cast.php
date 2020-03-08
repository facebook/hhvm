<?hh // strict

namespace NS_cast;

function main(): void {
  $v = 0;
  var_dump((bool)$v);
//  var_dump((boolean)$v);	// not permitted
  var_dump((int)$v);
//  var_dump((integer)$v);	// not permitted
  var_dump((float)$v);
//  var_dump((double)$v);	// not permitted
//  var_dump((real)$v);
  var_dump((string)$v);
//  var_dump((array)$v);	// not permitted
//  var_dump((object)$v);	// not permitted

//  var_dump((binary)$v);	// not permitted
//  var_dump((binary)"");	// not permitted
//  var_dump((binary)"abcdef");// not permitted

  var_dump($v);
//  var_dump((unset)$v);	// currently accepted, but does nothing. Support will be removed
//  var_dump($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();
