<?hh

function main() {
  $e = dict[];
  $one = dict[1 => "bar"];
  $two = dict[1 => "apple", "1" => "orange"];

  $e[1] = "red";
  $e["1"] = "green";

  $one["1"] = "foo";
  $two[] = "hello";

  var_dump($e, $one, $two);

  $a1 = dict[1 => "a"];
  $a2 = dict["1" => "a"];
  $a1[] = "b";
  $a2[] = "b";

  var_dump($a1, $a2);
}

main();
