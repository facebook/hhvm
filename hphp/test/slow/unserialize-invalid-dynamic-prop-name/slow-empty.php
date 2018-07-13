<?hh

class A { public $a = 1; }

// slow path: empty name
var_dump(unserialize('O:1:"A":1:{s:0:"";i:1;}'));
