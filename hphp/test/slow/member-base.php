<?hh

function append(inout $arr, $idx, $value) {
  if (!isset($arr[$idx])) {
    $arr[$idx] = varray[];
  }
  $arr[HH\array_key_cast($idx)][] = $value;
  return $arr[$idx];
}

<<__EntryPoint>>
function main() {
  $x = darray[];
  append(inout $x, 'a', 2);
  append(inout $x, 'a', 3);
  append(inout $x, 'b', varray[]);
  append(inout $x, 'b', varray[]);
  append(inout $x, 'b', varray[]);
  var_dump($x);
}
