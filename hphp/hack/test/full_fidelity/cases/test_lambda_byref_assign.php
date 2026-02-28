<?hh

$f = &() ==> { $x = 5; return $x; }; // error
$x = &$foo->bar?->baz; // error
$f = &$x; // error
$f = &my_func(); // error
$f = &$x->member; // error
$f = &$x->method(); // error
$f = &$xs[$x]; // error
$f = &${name}; // error
$f = &self::$mySelfThing; // error
$f = &$$s; // error
$f = &new C(); // error
$y = &$$$$$$x; // error
$x = &($foo->bar()); // error
$x = &(Arrays::slice(self::NORMALIZED_REQUIRED_COLUMN_NAMES, 0)); // error
$x = &(PHP\array_slice($app_alerts_array, -1)); // error
