<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  // from man 6 unix (BSD) or man 7 unix (Linux)
  $known_values = dict[
    'Linux' => 108,
    'Darwin' => 104,
  ];
  $expected = $known_values[PHP_OS];
  $actual = _OS\SUN_PATH_LEN;
  if ($expected === $actual) {
    print "OK.\n";
    return;
  }
  printf("Expected %d, got %d\n", $expected, $actual);
}
