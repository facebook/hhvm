<?hh

function main($n) {
  $p = Pair{array($n), "foobar"};
  while (--$n > 0) $p2 = clone $p;
}

ini_set('memory_limit', '100K');
main(100000);
echo 'pass\n';
