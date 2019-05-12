<?hh

class B<reify T> {}
function f($x): B<shape('x' => @int, 'y' => int)>{ return $x; }
<<__EntryPoint>> function main(): void {
f(new B<shape('x' => string, 'y' => int)>());       // warn
f(new B<shape('x' => string, 'y' => string)>());    // error
}
