<?hh // strict

function test(array<int> $arr): void {
  foreach ($arr as &$x) {
    $x = 0;
  }
}
