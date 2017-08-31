<?php

class Dtor {
  function __destruct() {
    echo "Dtor::__destruct\n";
  }
}

function main() {
  $e = dict[];
  $one = dict[1 => "bar"];
  $two = dict[1 => "apple", "1" => "orange"];
  $e[1] = "red";
  $e["1"] = "green";
  $one["1"] = "foo";
  $three = dict['a' => 'b', 'c' => 'd'];

  unset($e[1]);
  unset($one[1]);
  unset($two["1"]);
  unset($three['not-there']);
  var_dump($e, $one, $two, $three);

  echo "====================================================\n";
  $four = dict[1 => 'a', '2' => new Dtor, 3 => false];
  unset($four['2']);
  var_dump($four);
}

main();
