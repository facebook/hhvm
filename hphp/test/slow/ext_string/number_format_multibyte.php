<?php

$separators = [
  'multichar' => ['herp', 'derp'],
  'multibyte' => ['¡', '×']
];

var_dump(
  array_map(
    function($boom) {
      list($dec, $thou) = $boom;
      return number_format(1234.5678, 2, $dec, $thou);
    },
    $separators
  )
);
