<?php

error_reporting(E_ALL);

$bad_bases = array(
  null,
  0,
  1,
  0.0,
  1.0,
  false,
  true
);

foreach ($bad_bases as $bad_base) {
  echo "--Start--\n";
  var_dump($bad_base);

  // zend 5.5 doesn't warn on this access, but warns on all others
  $c = $bad_base[0];

  $bad_base[0] = 1;

  $result = ($bad_base[0] += 1);

  $result = ($bad_base[] = 1);

  echo "--End--\n\n";
}
