<?php

function reflect($name) {
  echo '==== ', $name, ' ====', "\n";
  $rf = new ReflectionFunction($name);
  var_dump($rf->getReturnTypeText());
  $params = $rf->getParameters();
  var_dump(count($params));
}

function main() {
  reflect('hash_algos');
}
main();
