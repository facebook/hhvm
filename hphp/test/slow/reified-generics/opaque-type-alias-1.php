<?hh

newtype A = int;
type B = int;
type A2 = (A, A);

class C<reify T> {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(new C<int>() is C<A>);
  var_dump(new C<A>() is C<int>);

  var_dump(new C<int>() is C<B>);
  var_dump(new C<B>() is C<int>);

  var_dump(new C<(A, A)>() is C<A2>);
  var_dump(new C<A2>() is C<(A, A)>);

  var_dump(new C<(int, int)>() is C<A2>);
  var_dump(new C<A2>() is C<(int, int)>);

  var_dump(type_structure('A'));
  var_dump(type_structure('B'));

  var_dump(type_structure('A2'));
}
