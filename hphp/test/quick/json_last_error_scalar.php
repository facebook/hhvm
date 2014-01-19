<?php
$data = [
  'null',
  'false',
  'true',
  '"abc"',
  '"ab\"c"',
  '0',
  '0.45',
  '-0.5',
  'invalid',
];

foreach($data as $str) {
  echo "JSON: $str\n";
  var_dump(json_decode($str));
  echo "Error: ", json_last_error(), "\n";
}
