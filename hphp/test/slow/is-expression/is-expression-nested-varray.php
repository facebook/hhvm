<?hh

<<__EntryPoint>>
function main() {
  var_dump(0 is (int, varray<_>));
  var_dump(tuple(0, vec[]) is (int, varray<_>));
  var_dump(tuple(0, dict[]) is (int, varray<_>));

  var_dump(0 is shape('a' => varray<_>));
  var_dump(shape('a' => vec[]) is shape('a' => varray<_>));
  var_dump(shape('a' => dict[]) is shape('a' => varray<_>));
}
