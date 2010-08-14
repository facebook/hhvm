<?php
if ($argc > 1) {
  $argc--;
  array_shift($argv);
  include $argv[0];
}
