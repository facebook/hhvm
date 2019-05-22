<?hh

class A { public $a = 1; }

// fast path: empty protected name
<<__EntryPoint>> function main(): void {
var_dump(unserialize('O:1:"A":2:{s:1:"a";i:1;s:3:"'."\0".'*'."\0".'";i:1;}'));
}
