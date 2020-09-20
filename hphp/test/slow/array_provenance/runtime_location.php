<?hh

function get_runtime_array() {
  $d0 = dict['a' => 17];
  $d1 = dict['b' => 34];
  try {
    var_dump($d0 < $d1);
  } catch (Exception $e) {
    return $e->getTrace();
  }
}

<<__EntryPoint>>
function main() {
  $arr0 = get_runtime_array();
  $p0 = HH\get_provenance($arr0);
  var_dump($p0);

  $options = dict['serializeProvenanceAndLegacy' => true];
  $arr1 = unserialize(HH\serialize_with_options($arr0, $options));
  $p1 = HH\get_provenance($arr1);
  var_dump($p1);

  var_dump($p0 === $p1);
}
