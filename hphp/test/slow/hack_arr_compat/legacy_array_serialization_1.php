<?hh

function mark_legacy_and_serialize($arr) {
  echo "\n";
  echo "before legacy bit set:\n" . serialize($arr). "\n";
  var_dump($arr);
  $arr = HH\enable_legacy_behavior($arr);
  echo "\nafter legacy bit set:\n" . serialize($arr) . "\n";
  var_dump($arr);
  echo "\n";
}

mark_legacy_and_serialize(dict[]);
mark_legacy_and_serialize(dict[
  "foo" => "quux",
  "bar" => vec[1, 2, 3],
  "baz" => dict[]
]);
mark_legacy_and_serialize(vec[]);
mark_legacy_and_serialize(vec[1, 2, 3, vec[1, 2, 3]]);
mark_legacy_and_serialize(array(1, 2, 3));
mark_legacy_and_serialize(varray[1, 2, 3]);
mark_legacy_and_serialize(darray["foo" => "bar"]);
