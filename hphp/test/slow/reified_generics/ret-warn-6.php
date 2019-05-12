<?hh

class B<reify T> {}
class C<reify Ta, reify Tb> {}
function f($x): B<C<@int, int>>{ return $x; }
<<__EntryPoint>> function main(): void {
f(new B<C<string, int>>());    // warn
f(new B<C<string, string>>()); // error since second tp is incorrect, even thought first tp is warn
}
