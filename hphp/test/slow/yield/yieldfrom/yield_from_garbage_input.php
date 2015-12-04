<?php

function gen($g) {
  yield from $g;
}


foreach(gen(8) as $val) {
  var_dump($val);
}
