<?hh // strict

function test(array<int> $arr): void {
  foreach ($arr as $_ => &$x) {
    $x = 0;
  }
}
