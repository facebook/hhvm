<?hh

function main() {
  $d1 = dict["1" => "x"];
  $d2 = dict["1" => "x", 1 => "y"];

  $s1 = serialize($d1);
  $s2 = serialize($d2);
  var_dump($s1, $s2);

  $o1 = unserialize($s1);
  $o2 = unserialize($s2);
  var_dump($o1, $o2);

  $o1[1] = "y";
  var_dump($o1["1"], $o1[1]);
}

main();
