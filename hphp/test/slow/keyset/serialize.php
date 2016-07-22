<?hh

function main() {
  $k1 = keyset["1", "x"];
  $k2 = keyset["1", "x", 1, "x"];

  $s1 = serialize($k1);
  $s2 = serialize($k2);
  var_dump($s1, $s2);

  $o1 = unserialize($s1);
  $o2 = unserialize($s2);
  var_dump($o1, $o2);

  $o1[] = "y";
  var_dump($o1, $o2);
  var_dump($o1["1"], $o2[1]);
}

main();
