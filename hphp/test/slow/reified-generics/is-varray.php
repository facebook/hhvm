<?hh

class C<reify T> {}

<<__EntryPoint>>
function main() :mixed{
  $c = new C<varray<int>>;
  var_dump($c is C<vec<_>>);
  var_dump($c is C<vec<int>>);
  var_dump($c is C<vec<string>>);
  var_dump($c is C<varray<_>>);
  var_dump($c is C<varray<int>>);
  var_dump($c is C<varray<string>>);

  print("\n");

  $c = new C<vec<int>>;
  var_dump($c is C<vec<_>>);
  var_dump($c is C<vec<int>>);
  var_dump($c is C<vec<string>>);
  var_dump($c is C<varray<_>>);
  var_dump($c is C<varray<int>>);
  var_dump($c is C<varray<string>>);
}
