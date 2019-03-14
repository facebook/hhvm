<?hh // strict

$f = &() ==> { $x = 5; return $x; }; // error
$x = &$foo->bar?->baz; // error

$f = &$x; // legal
$f = &my_func(); // legal
$f = &$x->member; // legal
$f = &$x->method(); // legal
$f = &$xs[$x]; // legal
$f = &${name}; // legal
$f = &self::$mySelfThing; // legal
$f = &$$s; // legal
$f = &new C(); // legal
$y = &$$$$$$x; // legal
$y = $foo->x(function() use (&$foo) { return $foo; }); // legal
$x = &($foo->bar()); // legal
$x = &(Arrays::slice(self::NORMALIZED_REQUIRED_COLUMN_NAMES, 0)); // legal
$x = &(PHP\array_slice($app_alerts_array, -1)); // legal
