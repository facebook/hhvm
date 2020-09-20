<?hh

<<__EntryPoint>>
function main() {
  var_dump(array_column(vec[varray[4, 5, 6]], '2'));
  var_dump(array_column(vec[darray[2 => 6]], '2'));
  var_dump(array_column(vec[vec[4, 5, 6]], '2'));
  var_dump(array_column(vec[dict[2 => 6]], '2'));
  var_dump(array_column(vec[keyset[2]], '2'));
}
