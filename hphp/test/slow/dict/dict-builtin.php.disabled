<?hh

function main() {
  $d = dict[1 => "apple", "1" => "orange", 2 => "tomato", "2" => "potato"];
  var_dump(array_map($v ==> strtoupper($v), $d));
  var_dump(array_filter($d, $v ==> $v != "tomato"));
}

main();
