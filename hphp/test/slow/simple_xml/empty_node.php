<?php

function main() {
  $a = simplexml_load_string("<root />");
  var_dump($a->unknown);
  var_dump((bool) $a->unknown);
  var_dump((int) $a->unknown);
  var_dump((string) $a->unknown);
  var_dump((double) $a->unknown);
  var_dump((array) $a->unknown);
  var_dump($a->unknown == null);
  var_dump(null == $a->unknown);
}
main();
