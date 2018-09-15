<?hh // strict

$f = &() ==> { static $x; return $x; }; // error
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
$x = &$foo->bar?->baz; // legal
