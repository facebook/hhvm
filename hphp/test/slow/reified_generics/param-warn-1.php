<?hh

class B<reify T> {}
class C<<<__Warn>> reify T> {}

function f(C<int> $_) { echo "done\n"; }

f(new C<int>());       // correct
f(new C<string>());    // only warn

function g(B<C<int>> $_) { echo "done\n"; }

g(new B<C<int>>());    // correct
g(new B<C<string>>()); // only warn

g(new B<string>());    // error
