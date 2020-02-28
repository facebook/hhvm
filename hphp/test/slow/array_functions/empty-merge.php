<?hh

function get_empty_array() {
  return extension_loaded('pdo') ? varray[] : varray['wut'];
}

function main() {
  $emp = get_empty_array();
  $full = varray[new stdClass, 2];
  $merge = array_merge($emp, $full);
  return $merge;
}


<<__EntryPoint>>
function main_empty_merge() {
var_dump(main());
}
