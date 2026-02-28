<?hh

class B<reify T> {}
class C<<<__Warn>> reify T> {}

function f(C<int> $_) :mixed{ echo "done\n"; }

function g(B<C<int>> $_) :mixed{ echo "done\n"; }
<<__EntryPoint>>
function entrypoint_paramwarn1(): void {

  f(new C<int>());       // correct
  f(new C<string>());    // only warn

  g(new B<C<int>>());    // correct
  g(new B<C<string>>()); // only warn

  g(new B<string>());    // error
}
