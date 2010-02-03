<?php
if ($argc > 1) {
  $argc--;
  array_shift($argv);
  include $argv[0];
} else {
  echo "Give me a file\n";
}
