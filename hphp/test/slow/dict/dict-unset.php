<?hh

function main() {
  $e = dict[];
  $one = dict[1 => "bar"];
  $two = dict[1 => "apple", "1" => "orange"];
  $e[1] = "red";
  $e["1"] = "green";
  $one["1"] = "foo";

  unset($e[1]);
  unset($one[1]);
  unset($two["1"]);
  var_dump($e, $one, $two);
}

main();
