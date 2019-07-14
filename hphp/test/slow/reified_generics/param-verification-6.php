<?hh

class B<reify T> {}

function f(?B<?int> $_) { echo "yep\n"; }
<<__EntryPoint>> function main(): void {
f(null);
f(new B<int>());
f(new B<?int>());
}
