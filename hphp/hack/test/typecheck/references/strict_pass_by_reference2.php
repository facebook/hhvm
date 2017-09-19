//// file1.php
<?hh

function array_extend(&$dst, $src) {
  foreach ($src as $x) {
    $dst[] = $x;
  }
}

//// file2.php
<?hh // strict

function test(array<int> $arr): array<int> {
  array_extend(&$arr, array(42, 999));
  return $arr;
}
