<?hh

type A = int;
class C<reify T> {}

<<__EntryPoint>>
function main() {
  var_dump(new C<int>() is C<A>);
  var_dump(new C<A>() is C<int>);
}
