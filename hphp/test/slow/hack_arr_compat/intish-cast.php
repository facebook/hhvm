<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(array_column(vec[vec[4, 5, 6]], '2'));
  var_dump(array_column(vec[dict[2 => 6]], '2'));
  var_dump(array_column(vec[vec[4, 5, 6]], '2'));
  var_dump(array_column(vec[dict[2 => 6]], '2'));
  var_dump(array_column(vec[keyset[2]], '2'));
}
