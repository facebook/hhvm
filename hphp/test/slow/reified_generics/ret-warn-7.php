<?hh

class B<reify Ta, reify Tb> {}
class C<reify T> {}
function f($x): B<C<@int>, int>{ return $x; }
<<__EntryPoint>> function main(): void {
f(new B<C<string>, int>());       // warn
f(new B<C<string>, string>()); // error since second tp is incorrect, even thought first tp is warn
}
