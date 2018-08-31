<?php


class someclass {
  static public function yo() {
    return !empty($_ENV['hey']);
  }
}

function asd() { return mt_rand() ? 'a' : 2; }
function main() {
  $time = asd();
  $items = array();
  if (someclass::yo()) {
    $items[] = 'ZZZZ';
  }
  $items[] = $time;
  return $items;
}

<<__EntryPoint>>
function main_array_possibly_empty_001() {
main();
}
