<?php

function gen() {
  yield from null;
}

foreach(gen() as $val) {
  echo "Iteration!\n";
  var_dump($val);
}

echo "Done\n";
