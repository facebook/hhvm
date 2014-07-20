<?php

function main() {
  $a = hphp_msarray();
  $a['field1'] = 10;
  $a['field2'] = 'url';

  foreach ($a as $k => $v) {
    // this is fine
    var_dump($k);
  }

  foreach ($a as &$v) {
    // warning
    $v = 'references';
  }
  var_dump($a);
}

main();
