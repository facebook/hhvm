<?hh

class B<reify T> {}
class C<<<__Warn>> reify T> {}

function f($x): C<int> { return $x; }

f(new C<int>());       // correct
f(new C<string>());    // only warn

function g($x): B<C<int>>{ return $x; }

g(new B<C<int>>());    // correct
g(new B<C<string>>()); // only warn

g(new B<string>());    // error
