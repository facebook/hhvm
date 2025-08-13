<?hh

<<__EntryPoint>>
async function main() {
  $usage = tc_usage();
  $keys = HH\Lib\Keyset\sort(HH\Lib\Keyset\keys($usage));
  var_dump($keys);
  foreach ($keys as $key) {
    $elem = $usage[$key];
    var_dump(HH\Lib\C\contains_key($elem, "used"));
    var_dump(HH\Lib\C\contains_key($elem, "capacity"));
    var_dump(HH\Lib\C\contains_key($elem, "global"));
  }
}

