<?php

function x($y, $k) {
  var_dump($k($y));
  var_dump(floor($y));
}


<<__EntryPoint>>
function main_floor_ceil() {
x(array(1,2,3,4), 'floor');
x(array(1,2,3,4), 'ceil');
}
