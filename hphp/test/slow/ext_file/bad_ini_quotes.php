<?php

function main() {
  $path = __DIR__.'/bad_ini_quotes.ini';
  $x = file_get_contents($path);
  $y = parse_ini_string($x, true);
  var_dump($y);
}

main();
