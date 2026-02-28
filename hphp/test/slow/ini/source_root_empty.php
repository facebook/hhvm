<?hh

<<__EntryPoint>>
function main_source_root_empty() :mixed{
$expected = getcwd();
if ($expected[strlen($expected)-1] !== '/') {
  $expected .= '/';
}

$actual = ini_get('hhvm.server.source_root');
var_dump(
  ($actual === $expected)
  ? "matches"
  : vec[$expected, $actual]
);
}
