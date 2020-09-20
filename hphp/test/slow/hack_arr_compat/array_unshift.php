<?hh

<<__EntryPoint>>
function main() {
  echo "Unshift dict\n";
  $a = dict["1" => 42];
  var_dump(array_unshift(inout $a, 2));
  var_dump($a);

  echo "Unshift darray\n";
  $a = darray[1 => 42];
  var_dump(array_unshift(inout $a, 2));
  var_dump($a);
  var_dump(is_darray($a));

  echo "Unshift vec\n";
  $a = vec[42];
  var_dump(array_unshift(inout $a, 2));
  var_dump($a);

  echo "Unshift varray\n";
  $a = varray[42];
  var_dump(array_unshift(inout $a, 2));
  var_dump($a);
  var_dump(is_varray($a));
}
