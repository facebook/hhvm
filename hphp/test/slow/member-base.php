<?hh

function append(inout $arr, $idx, $value) :mixed{
  if (!isset($arr[$idx])) {
    $arr[$idx] = vec[];
  }
  $arr[HH\array_key_cast($idx)][] = $value;
  return $arr[$idx];
}

<<__EntryPoint>>
function main() :mixed{
  $x = dict[];
  append(inout $x, 'a', 2);
  append(inout $x, 'a', 3);
  append(inout $x, 'b', vec[]);
  append(inout $x, 'b', vec[]);
  append(inout $x, 'b', vec[]);
  var_dump($x);
}
