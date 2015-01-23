<?php

function x($y, $k) {
  var_dump($k($y));
  var_dump(floor($y));
}

x(array(1,2,3,4), 'floor');
x(array(1,2,3,4), 'ceil');
