<?php


class c {
}

function main($a) {
  return $a[c::BAR];
}
var_dump(main(array('hello there' => 'success')));
