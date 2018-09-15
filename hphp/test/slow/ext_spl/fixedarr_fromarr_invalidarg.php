<?php


<<__EntryPoint>>
function main_fixedarr_fromarr_invalidarg() {
try {
  SplFixedArray::fromArray(['string'=>'string']);
} catch (InvalidArgumentException $e) {
  echo $e->getMessage();
  echo "\n";
}
}
