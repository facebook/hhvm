<?php

$format = '
  function %s($a, $b) {
    if (false) {} // force translation
    $x = $a %s $b;
    printf("%%s($a) %s %%s($b) = %%s($x)\n",
           gettype($a), gettype($b), gettype($x));
  }
  %s(%s, %s);';

class c {
  public function __toString() {
    return 'c';
  }
}

$ops = array(
  '&',
  '^',
  '|',
);

$values = array(
  'true',
  '42',
  '24.1987',
  '"str"',
  'array(1, 2, 3)',
  'new c()',
  'null',
);

for ($o = 0; $o < count($ops); ++$o) {
  for ($i = 0; $i < count($values); ++$i) {
    for ($j = 0; $j < count($values); ++$j) {
      $f_name = sprintf("f%d%d%d", $o, $i, $j);
      eval(sprintf($format, $f_name, $ops[$o], $ops[$o],
                   $f_name, $values[$i], $values[$j]));
    }
  }
}

function test_uninit() {
  if (false) {}
  $a = 'string';
  $x = $a & $b;
  var_dump($x);

  $a = 'string';
  $x = $a ^ $b;
  var_dump($x);
}

@test_uninit();
