<?hh

class B<reify T> {}
class C<reify T> {}

function f(B<@C<@int>> $_) { echo "done\n"; }
function g(B<C<@int>> $_) { echo "done\n"; }
<<__EntryPoint>> function main(): void {
f(new B<C<int>>());     // correct
f(new B<C<string>>());  // warn
f(new B<B<int>>());     // warn

g(new B<C<int>>());     // correct
g(new B<C<string>>());  // warn
g(new B<B<int>>());     // error
}
