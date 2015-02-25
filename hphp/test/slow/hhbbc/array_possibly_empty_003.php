<?php


class someclass {
  static public function yo() {
    return !empty($_ENV['hey']);
  }
}

function asd() { return mt_rand() ? 'a' : 2; }
function array_of_one() { return array(asd()); }
function main() {
  $time = asd();
  $items = array();
  if (someclass::yo()) {
    $items = array_of_one();
  }
  $items[1] = $time;
  return $items;
}
main();
