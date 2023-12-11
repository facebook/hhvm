<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize>> function get($k) :mixed{
  echo "get() called....\n";
  return $k;
}

function main() :mixed{
  var_dump(get(dict[]));
  var_dump(get(dict[]));

  var_dump(get(dict[100 => 100, 200 => 200, 300 => 300]));
  var_dump(get(dict[100 => 100, 200 => 200, 300 => 300]));

  var_dump(get(dict[300 => 300, 200 => 200, 100 => 100]));
  var_dump(get(dict[300 => 300, 200 => 200, 100 => 100]));

  var_dump(get(dict['value' => 'value', 'value2' => 'value2']));
  var_dump(get(dict['value' => 'value', 'value2' => 'value2']));

  var_dump(get(dict['value2' => 'value2', 'value' => 'value']));
  var_dump(get(dict['value2' => 'value2', 'value' => 'value']));

  var_dump(get(dict['value' => 'value']));
  var_dump(get(dict['value' => 'value']));

  var_dump(get(dict['value2' => 'value2']));
  var_dump(get(dict['value2' => 'value2']));

  var_dump(get(vec[]));
  var_dump(get(vec[]));
  var_dump(get(keyset[]));
  var_dump(get(Map{}));
  var_dump(get(Set{}));
  var_dump(get(Vector{}));

  var_dump(get(dict['value' => 'value', 'value2' => 'value2']));
  var_dump(get(keyset['value', 'value2']));
  var_dump(get(Map{'value' => 'value', 'value2' => 'value2'}));
  var_dump(get(Set{'value', 'value2'}));

  var_dump(get(vec['value', 'value2']));
  var_dump(get(vec['value', 'value2']));
  var_dump(get(Vector{'value', 'value2'}));
}

<<__EntryPoint>>
function main_memoize() :mixed{
main();
}
