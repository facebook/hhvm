<?hh

class A { public $a = 1; }

// fast path: empty private name
<<__EntryPoint>> function main(): void {
var_dump(unserialize('O:1:"A":2:{s:1:"a";i:1;s:11:"'."\0".'baseclass'."\0".'";i:1;}'));
}
