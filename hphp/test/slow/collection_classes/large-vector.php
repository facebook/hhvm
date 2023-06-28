<?hh
function main() :mixed{
  $x = Vector {};
  $x->resize(50000, null);
  for ($i = 50000; $i < 100000; ++$i) {
    $x[] = $i;
  }
  $a = $x->toVArray();
  foreach ($a as $k => $v) {
    if ($k % 5000 == 0) {
      echo ".";
    }
  }
  $b = $a;
  $b[] = 'foo';
  echo "Done\n";
}

<<__EntryPoint>>
function main_large_vector() :mixed{
main();
}
