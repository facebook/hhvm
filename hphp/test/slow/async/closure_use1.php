<?php

function foo($t) {
  $x = async function() use ($t) {
    await $t();
    $b = await $t();
    return 1 + $b;
  };

  var_dump($x()->join());
}

$f = async function() {
  return 42;
};

foo($f);
