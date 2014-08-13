<?php
function gmp_var_dump($data) {
  switch (gettype($data)) {
    case 'array':
      echo 'array(' . count($data) . ') {' . PHP_EOL;
      if (!empty($data)) {
        foreach ($data as $k => $v) {
          echo '  ["' . $k . '"]=>' . PHP_EOL;
          gmp_var_dump($v);
        }
      }
      echo '}' . PHP_EOL;
      break;
    case 'resource':
      echo 'Resource => ' . var_export(gmp_strval($data), true) . PHP_EOL;
      break;
    default:
      var_dump($data);
  }
}
