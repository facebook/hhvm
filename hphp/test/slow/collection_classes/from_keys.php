<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(Vector::fromKeysOf(dict['a' => 17, 'b' => 34]));
  var_dump(Vector::fromKeysOf(null));
  var_dump(Vector::fromKeysOf(1734));
}
