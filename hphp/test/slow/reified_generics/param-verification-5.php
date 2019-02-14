<?hh

class B<reify T> {}

function f(?B<int> $_) { echo "yep\n"; }
f(null);
f(new B<int>());
f(new B<?int>());
