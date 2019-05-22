<?hh

class A { public $a = 1; }

// slow path: malformed private/protected name
<<__EntryPoint>> function main(): void {
var_dump(unserialize('O:1:"A":1:{s:4:"'."\0".'bad";i:1;}'));
}
