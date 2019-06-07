<?hh
function array_map_recursive($callback, $array) {
  foreach ($array as $key => $value) {
    if (is_array($array[$key])) {
      $array[$key] = array_map_recursive($callback, $array[$key]);
    } else {
      $array[$key] = call_user_func($callback, $array[$key]);
    }
  }
  return $array;
}

function openssl_pkey_get_bin2hex_details($res) {
  return array_map_recursive($v ==> bin2hex((string)$v), openssl_pkey_get_details($res));
}
