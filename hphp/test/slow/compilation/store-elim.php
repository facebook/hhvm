<?hh

function main($a, $b, $c) :mixed{
  $i = 0;
  if ($c) {
    $t = 0;
  } else {
    $t = 1;
  }
  if ($i & 1) {
    $t = 2;
  }
  while (++$i < $a) {
    if ($i & 1) {
      $t = 2;
    }
  }

  if ($b == 42) {
    $t = 4;
  }

  var_dump($t);
}


<<__EntryPoint>>
function main_store_elim() :mixed{
for ($i = 0; $i < 2; $i++) {
  main(1, 42, 0);
  main(2, 42, 0);
  main(1, 43, 0);
  main(2, 43, 0);

  main(1, 42, 1);
  main(2, 42, 1);
  main(1, 43, 1);
  main(2, 43, 1);
}
}
