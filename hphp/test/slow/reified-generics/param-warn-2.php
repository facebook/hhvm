<?hh

class B<reify T> {}
class C<reify T> {}

function f(C<<<__Soft>> int> $_) :mixed{ echo "done\n"; }

function g(B<C<<<__Soft>> int>> $_) :mixed{ echo "done\n"; }
<<__EntryPoint>>
function entrypoint_paramwarn2(): void {

  f(new C<int>());       // correct
  f(new C<string>());    // only warn

  g(new B<C<int>>());    // correct
  g(new B<C<string>>()); // only warn

  g(new B<string>());    // error
}
