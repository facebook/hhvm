<?php
$expected = getcwd();
if ($expected[strlen($expected)-1] !== '/') {
  $expected .= '/';
}

$actual = ini_get('hhvm.server.source_root');
var_dump(
  ($actual === $expected)
  ? "matches"
  : [$expected, $actual]
);
