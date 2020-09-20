<?hh

class B<reify T> {}
class C<reify T> {}

function f($x): C<<<__Soft>> int> { return $x; }

function g($x): B<C<<<__Soft>> int>>{ return $x; }
<<__EntryPoint>>
function entrypoint_retwarn3(): void {

  f(new C<int>());       // correct
  f(new C<string>());    // only warn

  g(new B<C<int>>());    // correct
  g(new B<C<string>>()); // only warn

  g(new B<string>());    // error
}
