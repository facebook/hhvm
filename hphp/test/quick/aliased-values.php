<?hh

<<__NEVER_INLINE>>
function test($x) :mixed{
  $arr = dict["bar" => 17];
  $y = idx($arr, HH\array_key_cast($x), $x);
  if ($y === "bar") return true;
  return array_key_exists(HH\array_key_cast($y), $arr);
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(test(json_decode('"foo"')));
}
