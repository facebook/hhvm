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
main();
