<?php

function main() {
  var_dump(Collator::SORT_NUMERIC);
  var_dump(DateTime::RFC822);
  var_dump(Memcached::HAVE_JSON);
  var_dump(PDO::ERR_NONE);
  echo "\n";
  var_dump(constant('Collator::SORT_NUMERIC'));
  var_dump(constant('DateTime::RFC822'));
  var_dump(constant('Memcached::HAVE_JSON'));
  var_dump(constant('PDO::ERR_NONE'));
}
main();

