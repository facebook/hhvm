<?hh
function main() {
  // Test arrays whose size exceeds 65536
  $a = array_pad(array(), 50000, null);
  for ($i = 50000; $i < 100000; ++$i) {
    $a[] = $i;
  }
  foreach ($a as $k => $v) {
    if ($k % 5000 == 0) {
      echo ".";
    }
  }
  $b = $a;
  $b[] = 'foo';
  echo "Done\n";
}
main();
