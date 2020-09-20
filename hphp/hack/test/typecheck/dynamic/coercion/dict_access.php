<?hh // strict

function foo(dict<int, string> $dic, dynamic $d): string {
  return $dic[$d];
}
