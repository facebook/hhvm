<?hh

class A { public $a = 1; }

// slow path: empty private name
var_dump(unserialize('O:1:"A":1:{s:11:"'."\0".'baseclass'."\0".'";i:1;}'));
